#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "hid.h"
#include "functions.h"
#include "custom_keymap.h"
#include "config.h"
#include "sysutils.h"

appcontext_reporter_t &appctxReporter() {
  static appcontext_reporter_t ctx = {
    F_Key(),
    F_Mouse(),
    F_Touchpad(),
  };

  return ctx;
}

bool FunctionHolder::push(Functions *f, FunctionCode_t fc) {
  if(f == nullptr) {
    LOG_ERROR("Function is NULL");
    return false;
  }

  if(fc != F_NONE) { // duplicate fcode not allowed
    for(FunctionPriority p = F_PR_LOW; p < F_PR_MAX; p = FunctionPriority(p + 1)) {
      uint8_t i = 0;
      while(i < count[p]) {
        if(fcode[p][i] == fc) {
#if defined(DEBUG)
          if(ctxsys.kb_ctrl.isDebugMode()) {
            LOG_DEBUG("Duplicate fcode 0x", DebugLogBase::HEX, fc, "at priority", p);
          }
#endif
          return false;
        }
        i++;
      }
    }
  }

  FunctionPriority priority = f->getPriority();
  if(count[priority] >= MAX_QUEUE_SIZE) {
    LOG_ERROR("Function queue is full for type: ", priority);
    return false;
  }

  list[priority][count[priority]] = f;
  fcode[priority][count[priority]] = fc;
  count[priority]++;
  f->initLifecycle();

#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("Added to Q, Function 0x", DebugLogBase::HEX, f->getFunctionCode(), ", priority ", priority, ", count ", DebugLogBase::DEC, count[priority]);
  }
#endif
  return true;
}

void FunctionHolder::action(FunctionPriority priority) {
  unsigned long now = millis();
  uint8_t i;

  switch(priority) {
    case F_PR_HIGH:
    case F_PR_MIDDLE:
    case F_PR_LOW:
      i = 0;
      while(i < count[priority]) {
        Functions *f = list[priority][i];
        FunctionCode_t fc = fcode[priority][i];
#ifdef DEBUG
        if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
          LOG_DEBUG("Function designated 0x", DebugLogBase::HEX, fc, ", have 0x", f->getFunctionCode(), ", on priority ", priority, ", ", ", i ", DebugLogBase::DEC, i);
        }
#endif
        f->action(now, fc);
        if(f->getLifecycle() == F_LC_ONCE || f->getFunctionCode() != fc) { // case F_LC_ONCE
          remove(priority, i);
        } else {
          i++; // do not increment if removed.
        }
      }
      break;

    default:
      LOG_ERROR("Invalid priority: ", priority);
      break;
  }
}

bool FunctionHolder::remove(uint8_t priority, uint8_t index) {
  if(priority >= F_PR_MAX || index >= count[priority]) return false;
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("Remove Function 0x", DebugLogBase::HEX, list[priority][index]->getFunctionCode(), ", priority ", priority, ", count", DebugLogBase::DEC, count[priority], "-1");
  }
#endif
  list[priority][index] = list[priority][count[priority] - 1];
  fcode[priority][index] = fcode[priority][count[priority] - 1];
  count[priority]--;
  return true;
}

ActionResult F_ChangeProfile::childaction(unsigned long ms, FunctionCode_t fc) {
  if(!beginning) return F_RES_NONE;

  switch(profile) {
    case PROFILE_DEFAULT:
    case 0:
    case 1:
    case 2:
    case 3:
      break;
    default:
      LOG_ERROR("Unknown profile: ", profile);
      return F_RES_FAIL;
  }

  processed_time = ms;
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("Change profile to ", profile);
  }
#endif
  if(Config::loadConfiguration(profile)) return F_RES_SUCCESS;
  return F_RES_FAIL;
}


ActionResult F_KeyMacro::childaction(unsigned long ms, FunctionCode_t fc) {
  //if(!beginning) return F_RES_NONE; 
  if(!running) {
#ifdef DEBUG
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Macro start", fcode);
    }
#endif
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, ctxkeymap.keymap.getFunctionDescr(fcode), "start");
    running = true;
    processed_time = ms;
    entry_index = 0;
    code_index = 0;
  }

  bool finished = true; // temporarily true
  unsigned long passed = ms - processed_time;
  if(entry_index < seq.num) {
    finished = false;
    if(seq.entry[entry_index].num == code_index) {
      // just wait duration
      if(passed < seq.entry[entry_index].duration) {
        // do nothing
      } else {
        entry_index++;
        code_index = 0;
        processed_time = ms;
      }
    }
    
    else { // code_index < seq.entry[entry_index].num
      if(passed < seq.entry[entry_index].duration) {
        if(seq.entry[entry_index].simultaneous) {
          uint8_t i = 0;
          while(i < seq.entry[entry_index].num) {
            F_Key *key = (F_Key *)ctxkeymap.keymap.getFunctionInstance(seq.entry[entry_index].fcode[i]);
            ctxhid.hidmanager.addKeycode(key->getCode(), key->getMod());
            i++;
          }
        } else {
          F_Key *key = (F_Key *)ctxkeymap.keymap.getFunctionInstance(seq.entry[entry_index].fcode[code_index]);
          ctxhid.hidmanager.addKeycode(key->getCode(), key->getMod());
        }
      } else {
        if(seq.entry[entry_index].simultaneous) {
          entry_index++;
          code_index = 0;
          processed_time = ms;
        } else {
          code_index++;
          processed_time = ms;
        }
      }
    }
  }

  if(finished) {
#ifdef DEBUG
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Macro end", fcode);
    }
#endif
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, ctxkeymap.keymap.getFunctionDescr(fcode), "finished");
    running = false;
    lifecycle = F_LC_ONCE;
  }
  return F_RES_SUCCESS;
}

bool F_KeyMacro::addSequence(unsigned long duration, bool simul, std::initializer_list<FunctionCode_t> fcodes) {
  MacroEntry entry;
  entry.duration = duration;
  entry.num = 0;
  for(auto fcode : fcodes) {
    if(entry.num >= MACRO_MAXFUNCS) break;
    entry.fcode[entry.num++] = fcode;
  }

  return(addSequence(duration, simul, entry.fcode, entry.num));
}

bool F_KeyMacro::addSequence(unsigned long duration, bool simul, FunctionCode_t* fcodes, size_t num_codes) {
  if(seq.num >= MACRO_MAXENTRIES) {
    LOG_ERROR("Macro sequence is full: num ", seq.num);
    return false;
  }

  if(duration > 60000 || num_codes > MACRO_MAXFUNCS) {
    LOG_ERROR("Invalid parameters: duration ", duration, ", num_codes ", num_codes);
    return false;
  }

  seq.entry[seq.num].duration = duration;
  seq.entry[seq.num].simultaneous = simul;
  seq.entry[seq.num].num = num_codes;
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("Add macro duration", duration, ", simultaneous", simul, ", num", num_codes);
  }
#endif
  if(num_codes > 0) {
    uint8_t i = 0;
    while(i < num_codes) {
      FunctionCode_t fcode = fcodes[i];
      if(!(FunctionCodeTypeKey(fcode) || FunctionCodeTypeModifier(fcode))) {
        LOG_ERROR("function not allowed in macro: 0x", DebugLogBase::HEX, fcode);
        return false;
      }
#ifdef DEBUG
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG(" add sequence", i, ", fcode 0x", DebugLogBase::HEX, fcode);
      }
#endif
      seq.entry[seq.num].fcode[i++] = fcode;
    }
  }

  seq.num++;
  return true;
}

bool F_KeyMacro::validate(MacroSequence &ms) {
  bool f = true;

  if(ms.num > MACRO_MAXENTRIES) {
    ms.num = MACRO_MAXENTRIES;
    f = false;
    LOG_ERROR("Macro entries", ms.num, ">= MACRO_MAXENTRIES, set to ", MACRO_MAXENTRIES);
  }
  uint8_t s = 0;
  while(s < ms.num) {
    if(ms.entry[s].num > MACRO_MAXFUNCS) {
      ms.entry[s].num = MACRO_MAXFUNCS;
      f = false;
      LOG_ERROR("Macro entry", s, ", function num", ms.entry[s].num, ">= MACRO_MAXFUNCS, set to ", MACRO_MAXFUNCS);
    }
    uint8_t i = 0;
    while(i < ms.entry[s].num) {
      if(!(FunctionCodeTypeKey(ms.entry[s].fcode[i]) || FunctionCodeTypeModifier(ms.entry[s].fcode[i]))) {
        ms.entry[s].fcode[i] = 0;
        f = false;
        LOG_ERROR("Macro entry", s, ", function", ms.entry[s].fcode[i], "not allowed, set to 0");
      }
      i++;
    }
    s++;
  }

  return f;
}


ActionResult F_Mouse::childaction(unsigned long ms, FunctionCode_t fc) {
  // not necessary 'beginning'

  ActionResult r = F_RES_NONE;
  bool mouse_stop = false;

  switch(fc) {
    case F_NONE:
      if(button == 0 && xdelta == 0 && ydelta == 0) {
        if(button_prev == 0 && xdelta_prev == 0 && ydelta_prev == 0) break;
        mouse_stop = true;
      }

      if(mouse_stop) {
        if(!stopped) {
          ctxhid.hidmanager.mouseSendReport(0, 0, 0, 0, 0);
        }
        stopped = true;
      } else if(wheel) {
        ctxhid.hidmanager.mouseSendReport(button, 0, 0, ydelta, xdelta);
      } else {
        ctxhid.hidmanager.mouseSendReport(button, xdelta, ydelta, 0, 0);
      }

      button_prev = button;
      xdelta_prev = xdelta;
      ydelta_prev = ydelta;
      button = 0;
      xdelta = 0;
      ydelta = 0;
      r = F_RES_SUCCESS;
      break;

    case F_M_WHEEL:
      wheel = true;
      r = F_RES_SUCCESS;
      break;

    case F_MOUSE_MOVE:
      {
      Movement mv;
      mv.value32 = getInputValue32();
      xdelta = mv.x;
      ydelta = mv.y;
      r = F_RES_SUCCESS;
      }
      break;
    
    case F_MBTN_LEFT:
      button |= LEFT;
      r = F_RES_SUCCESS;
      break;

    case F_MBTN_RIGHT:
      button |= RIGHT;
      r = F_RES_SUCCESS;
      break;

    case F_MBTN_MIDDLE:
      button |= MIDDLE;
      r = F_RES_SUCCESS;
      break;

    case F_MBTN_BW:
      button |= BACKWARD;
      r = F_RES_SUCCESS;
      break;

    case F_MBTN_FW:
      button |= FORWARD;
      r = F_RES_SUCCESS;
      break;

    case F_JIGGLER:
      testCircle();
      r = F_RES_SUCCESS;
      break;

    default: // F_MBTN_XX
      LOG_ERROR("unknown FunctionCode:", DebugLogBase::HEX, fcode);
      r = F_RES_FAIL;
  }

  return r;
}

void F_Mouse::testCircle() {
  static uint16_t c = 0;
  float angle = PI * 2 * c / TEST_Hz;
  ctxhid.hidmanager.mouseSendReport(0, sin(angle)*100+100, cos(angle)*100+100, 0, 0);
  c = (c + 1) % TEST_Hz;
}

ActionResult F_Key::childaction(unsigned long ms, FunctionCode_t fc) {
  // not necessary 'beginning'

  ActionResult r = F_RES_NONE;

  if(fcode == F_NONE) {
    /*
    if(ctxkeymap.keymap.isTapAdvanced()) {
      if(keycode_size == 0) {
#ifdef DEBUG
        if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
          LOG_DEBUG("SKIP modifier", DebugLogBase::HEX, modifier);
        }
#endif
        modifier = 0;
      }
    }
    */

    if(ctxhid.hidmanager.isKeycodesChanged()) ctxhid.hidmanager.keyboardSendReport();
    ctxhid.hidmanager.clearKeycode(); // clear for next cycle.
    ctxsys.ev_handler.setKeycode(keycode_forevent, modifier_forevent);
    keycode_forevent = 0;
    modifier_forevent = 0;
    r = F_RES_SUCCESS;

  } else if(fcode == F_KBDPROTO) {
    if(!beginning) return F_RES_NONE;
    toggleKeyboardProtocolFeature();
  } else { // add keycode to HID report buffer.
    keycode_forevent = code;
    modifier_forevent = mod;
    if(ctxhid.hidmanager.addKeycode(code, mod)) r = F_RES_SUCCESS;
    else r = F_RES_FAIL;
  }

  return r;
}

void F_Key::setKeyboardProtocolFeature(bool feature) {
  HidManager::kbd_protocol_feature = feature;
#ifdef DEBUG
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Set keyboard protocol ->", (feature ? "Feature" : "Boot"));
    }
#endif
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1,
    "KBD Proto", (feature ? "Feature" : "Boot"));
}

bool F_Key::toggleKeyboardProtocolFeature() {
  setKeyboardProtocolFeature(!HidManager::kbd_protocol_feature);
  return HidManager::kbd_protocol_feature;
}

void F_Key::clearHidReport() {
  ctxhid.hidmanager.keyboardSendReport();
}

void F_Mouse::clearHidReport() {
  ctxhid.hidmanager.mouseSendReport(0, 0, 0, 0, 0);
}

ActionResult F_TapAdvanced::childaction(unsigned long ms, FunctionCode_t fc) {
  if(!beginning) return F_RES_NONE;

  ctxkeymap.keymap.enableTapAdvanced(!ctxkeymap.keymap.isTapAdvanced());
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("Tap advanced behavior ", (ctxkeymap.keymap.isTapAdvanced() ? "ON" : "OFF"));
  }
#endif
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1,
    "Tap Advanced", (ctxkeymap.keymap.isTapAdvanced() ? "ON" : "OFF"));
  return F_RES_SUCCESS;
}

void Functions::beginAllInstances() {
  auto &i = getInstances();
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("Instances:", i.size());
  }
#endif
  for(auto* instance : i) {
    if(instance) {
      instance->begin();
    }
  }
}

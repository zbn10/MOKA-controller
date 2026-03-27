#include <Arduino.h>
#include <Stream.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
//#include <GFXfont.h>
#include "fonts/ufbdf.h"
#include "fonts/spleen-12x24.h"
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "config.h"
#include "i2c_control.h"
#include "mcu.h"
#include "keysw_statemachine.h"
#include "custom_keymap.h"
#include "hid.h"

appcontext_sys_t &appctxSys() {
  static KeyboardControl kb_ctrl;
  static F_EventHandler ev_handler;
  static F_SystemLED f_systemled(DEFS_PIN_SYSTEM_LED, Config::getActiveKeyboardLED(), Config::getActiveLEDAttributes());
  static OLED oled(F_CHG_OLED, 128, 32, &Wire2nd, -1, DEFS_I2C2NDCLOCK);
  static appcontext_sys_t ctxsys{kb_ctrl, ev_handler, f_systemled, oled};
  return ctxsys;
}


void SystemEvent::setEvent(uint8_t pr, uint8_t gr, int8_t l, uint8_t t, const char *m1, const char *m2, const char *(*f1)(void), const char *(*f2)(void)) {
  priority = pr;
  group = gr; // if !GR_NONE, check duplicate in error Q
  life = l;

  if(t == SystemEvent::TYPE_TEXT) {
    type = t;
    func1 = func2 = nullptr;
    if(m1 != nullptr) {
      strncpy(msg1, m1, sizeof(msg1) - 1);
      msg1[sizeof(msg1) - 1] = '\0';
    } else msg1[0] = '\0';
    if(m2 != nullptr) {
      strncpy(msg2, m2, sizeof(msg2) - 1);
      msg2[sizeof(msg2) - 1] = '\0';
    } else msg2[0] = '\0';

  } else if(t == SystemEvent::TYPE_FUNC) {
    type = t;
    func1 = f1;
    func2 = f2;
    msg1[0] = '\0'; msg2[0] = '\0';

  } else {
    LOG_ERROR("unknown event type", t);
    type = 0;
    msg1[0] = '\0'; msg2[0] = '\0';
    func1 = func2 = nullptr;
  }
}

void SystemEventQ::initQ(SystemEvent *events_array, uint8_t size) {
  if(events_array == nullptr || size == 0) {
    head = tail = nullptr;
    count = 0;
    return;
  }

  count = size;
  head = events_array;
  tail = &events_array[size - 1];
  events_array[0].setPrev(nullptr);
  for(uint8_t i = 0; i < size - 1; i++) {
    events_array[i].setID(i + 1);
    events_array[i].setNext(&events_array[i+1]);
    events_array[i+1].setPrev(&events_array[i]);
  }
  events_array[size - 1].setID(size - 1);
  events_array[size - 1].setNext(nullptr);
  return;
}

// DUP_IGNORE: allow duplicates
//
// return
// nullptr: success ot pushback
// SystemEvent *event: ejected object due to duplicate
SystemEvent *SystemEventQ::pushBack(SystemEvent *ev, uint8_t check_duplicate) {
  if(ev == nullptr) return nullptr;

  if(check_duplicate >= DUP_MAX) {
    LOG_ERROR("invalid check_duplicate value: ", check_duplicate);
    return ev;
  }

  if(check_duplicate != DUP_IGNORE && ev->getGroup() != SystemEvent::GR_NONE) {
    SystemEvent *n = head;
    while(n != nullptr) {
      if(n->getGroup() == ev->getGroup()) {
#if defined(DEBUG)
        if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
          LOG_DEBUG("group", ev->getGroup(), "exists in Q. ev->priority", ev->getPriority(), ", ev->id", ev->getID());
        }
#endif
        switch(check_duplicate) {
          case DUP_PROHIBIT: // first to win
            return ev;
      
          case DUP_OVERWRITE: // last to win
            //replace(n, ev);
            ev->setPrev(n->getPrev());
            ev->setNext(n->getNext());
            if(ev->getPrev() != nullptr) {
              (ev->getPrev())->setNext(ev);
            } else {
              head = ev;
            }
            if(ev->getNext() != nullptr) {
              (ev->getNext())->setPrev(ev);
            } else {
              tail = ev;
            }
            return n;
            break;
        }
      }
      n = n->getNext();
    }
  }

  ev->setPrev(tail);
  ev->setNext(nullptr);
  tail = ev;
  if(ev->getPrev() != nullptr) {
    (ev->getPrev())->setNext(ev);
  }
  if(head == nullptr) {
    head = ev;
  }

  count++;
  return nullptr;
}

SystemEvent *SystemEventQ::popFront() {
  if(head == nullptr) return nullptr;

  SystemEvent *ev = head;
  head = ev->getNext();
  if(head != nullptr) {
    (head)->setPrev(nullptr);
  } else {
    tail = nullptr;
  }

  count--;
  return ev;
}



void F_EventHandler::initSystemQ() {
  // Logo Text
  pushNewEvent(SystemEvent::PR_INFO, SystemEvent::GR_NONE, -1, DEFS_OLED_TEXT1, DEFS_OLED_TEXT2);
  pushNewEvent(SystemEvent::PR_INFO, SystemEvent::GR_NONE, -1, getKeymapProfile, getKeymapOSLayout);
#if defined(DEBUG)
  pushNewEvent(SystemEvent::PR_DEBUG, SystemEvent::GR_NONE, -1, getKbdMode, getTPMode);
  pushNewEvent(SystemEvent::PR_DEBUG, SystemEvent::GR_NONE, -1, "Firmware " MF_VERSION_STR, "Board    " MB_VERSION_STR);
  pushNewEvent(SystemEvent::PR_DEBUG, SystemEvent::GR_NONE, -1, getKeyCode, getKeyMod);
#if defined(SUPPORT_PHOTOIC_INFRARED)
  pushNewEvent(SystemEvent::PR_DEBUG, SystemEvent::GR_NONE, -1, getPhotoIC, getInfrared);
#endif
#endif

  // Logo Graphic
  // not implemented yet
}

void F_EventHandler::dumpSystemQ() {
  char buf[200];
  uint8_t pos;
  uint8_t n;

  if(current_event != nullptr) {
    pos = 0;
    APPEND_STR(buf, pos, "current event: %d:%d", current_event->getID(), current_event->getLife());
    LOG_DEBUG(buf);
  }

  SystemEventQ *qlist[] = { &empty_q, &priority_q, &error_q, &info_debug_q };
  const char *qlist_name[] = { "empty", "priority", "error", "info_debug" };
  for(uint8_t i = 0; i < sizeof(qlist)/sizeof(qlist[0]); i++) {
    SystemEventQ *q = qlist[i];
    n = q->getCount();
    pos = 0;
    APPEND_STR(buf, pos, "%s %d:", qlist_name[i], n);
    while(n--) {
      SystemEvent *ev = q->popFront();
      APPEND_STR(buf, pos, "id:%d, life:%d, pr:%d,", ev->getID(), ev->getLife(), ev->getPriority());
      q->pushBack(ev);
    }
    LOG_DEBUG(buf);
  }
}

bool F_EventHandler::pushNewEvent(uint8_t pr, uint8_t gr, int8_t life, const char *msg1, const char *msg2, uint8_t check_duplicate) {
  if(msg1 == nullptr && msg2 == nullptr) {
    LOG_ERROR("both msg1 and msg2 are null");
    return false;
  }
  SystemEvent *ev = empty_q.popFront();
  if(ev == nullptr) {
    LOG_ERROR("no event queue available");
    return false;
  }
  ev->setEvent(pr, gr, life, SystemEvent::TYPE_TEXT, msg1, msg2, nullptr, nullptr);
  return(pushNewEventInternal(ev, check_duplicate));
}

bool F_EventHandler::pushNewEvent(uint8_t pr, uint8_t gr, int8_t life, const char *(*func1)(void), const char *(*func2)(void), uint8_t check_duplicate) {
  if(func1 == nullptr && func2 == nullptr) {
    LOG_ERROR("both func1 and func2 are null");
    return false;
  }
  SystemEvent *ev = empty_q.popFront();
  if(ev == nullptr) {
    LOG_ERROR("no event queue available");
    return false;
  }
  ev->setEvent(pr, gr, life, SystemEvent::TYPE_FUNC, nullptr, nullptr, func1, func2);
  return(pushNewEventInternal(ev, check_duplicate));
}

// used for new event push
// return
// true: success to push, or success to remove preceding same group event
// false: fail or some error. ev moved to empty_q
// 
// responsible for placing *ev to any Q
bool F_EventHandler::pushNewEventInternal(SystemEvent *ev, uint8_t check_duplicate) {
  if(ev == nullptr) {
    LOG_ERROR("event is NULL");
    return false;
  }

  if(check_duplicate >= SystemEventQ::DUP_MAX) {
    LOG_ERROR("invalid check_duplicate value: ", check_duplicate);
    empty_q.pushBack(ev);
    return false;
  }

  uint8_t gr = ev->getGroup();
  uint8_t pr = ev->getPriority();
  int8_t life = ev->getLife();
  if(gr >= SystemEvent::GR_MAX || pr >= SystemEvent::PR_MAX ) {
    LOG_ERROR("invalid group", (int)gr, " or priority", (int)pr, ", id", (int)ev->getID(), ", life", (int)life);
    empty_q.pushBack(ev);
    return false;
  }

  // check current_event
  if(current_event && gr != SystemEvent::GR_NONE && gr == current_event->getGroup() && pr == current_event->getPriority()) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
      LOG_DEBUG("group", ev->getGroup(), "exists in current_event. ev->priority", ev->getPriority(), ", ev->id", ev->getID());
    }
#endif
    switch(check_duplicate) {
      case SystemEventQ::DUP_IGNORE:
        break;
        
      case SystemEventQ::DUP_PROHIBIT:
        empty_q.pushBack(ev);
        return false;

      case SystemEventQ::DUP_OVERWRITE:
        {
        current_event = empty_q.pushBack(current_event);
        SystemEventQ &q = getEventQByPriority(pr);
        SystemEvent *n = q.pushBack(ev, SystemEventQ::DUP_OVERWRITE);
        if(n) empty_q.pushBack(n);
        }
        return true;
    }
  }

  // check target Q
  /*
    debug event is set to InfoQ.
    popFront() handles DEBUG
  */
  SystemEventQ &q = getEventQByPriority(pr);
  SystemEvent *n = q.pushBack(ev, check_duplicate);
  if(n) empty_q.pushBack(n);
  return true;
}

// current_event -> empty_q if life ended,
// but new current_event is not set.
// if life > 0 continue, if life <= 0 next event.
// if force_transition is true, move to next event regardless of life.
//
// return
// true: still alive
// false: life ended or some error, event moved to other Q
bool F_EventHandler::updateLifeOfCurrentEvent(bool force_transition) {
  if(current_event == nullptr) return false;

  /*
  force_transition:
          false true
  life >0 alive next
      ==0 next  next -> to emptyQ
       <0 next  next
  */
  int8_t l = current_event->updateLife();
  if(l > 0 && !force_transition) return true;

  if(l == 0) {
    empty_q.pushBack(current_event);
  } else {
    SystemEventQ &q = getEventQByPriority(current_event->getPriority());
    SystemEvent *n = q.pushBack(current_event, SystemEventQ::DUP_PROHIBIT); // if new event exists with same group, use it.
    if(n) empty_q.pushBack(n);
  }
  current_event = nullptr;

  return false;
}

const char *F_EventHandler::getKeyCode() {
  static char keycode_buf[32];
  snprintf(keycode_buf, sizeof(keycode_buf), "Keycode  0x%02X", keycode);
  keycode_buf[sizeof(keycode_buf)-1] = '\0';
  return((const char *)keycode_buf);
}
const char *F_EventHandler::getKeyMod() {
  static char keymod_buf[32];
  snprintf(keymod_buf, sizeof(keymod_buf), "Modifier 0x%02X", modifier);
  keymod_buf[sizeof(keymod_buf)-1] = '\0';
  return((const char *)keymod_buf);
}

const char *F_EventHandler::getKeymapProfile() {
  static char buf[32];
  snprintf(buf, sizeof(buf), "Prof P:%c, L:%c", Config::getPrimaryProfileChar(), Config::getLoadedProfileChar());
  buf[sizeof(buf)-1] = '\0';
  return((const char *)buf);
}

const char *F_EventHandler::getKeymapOSLayout() {
  static char buf[32];
  snprintf(buf, sizeof(buf), "OSLayout %s", ctxkeymap.keymap.getOSLayoutStr());
  buf[sizeof(buf)-1] = '\0';
  return((const char *)buf);
}

const char *F_EventHandler::getKbdMode() {
  static char buf[32];
  snprintf(buf, sizeof(buf), "KBD Proto %s", HidManager::getKeyboardProtocolFeature() ? "NKRO" : "BOOT");
  buf[sizeof(buf)-1] = '\0';
  return((const char *)buf);
}

const char *F_EventHandler::getTPMode() {
  static char buf[32];
  snprintf(buf, sizeof(buf), "TP Mode %s", F_Touchpad::getMouseMode() ? "Mouse" : "Touchpad");
  buf[sizeof(buf)-1] = '\0';
  return((const char *)buf);
}

#if defined(SUPPORT_PHOTOIC_INFRARED)
const char *F_EventHandler::getPhotoIC() {
  snprintf(photoic_buf, sizeof(photoic_buf), "PhotoIC  %d", ctxksm.photoic_sw.getValue());
  photoic_buf[sizeof(photoic_buf)-1] = '\0';
  return((const char *)photoic_buf);
}
const char *F_EventHandler::getInfrared() {
  snprintf(infrared_buf, sizeof(infrared_buf), "Infrared %d", ctxksm.infrared_sw.getAnyValue());
  infrared_buf[sizeof(infrared_buf)-1] = '\0';
  return((const char *)infrared_buf);
}
#endif

ActionResult F_EventHandler::drawOLED(SystemEvent *ev) {
  if(ev == nullptr) {
    return F_RES_FAIL;
  }

  switch(ev->getType()) {
    case SystemEvent::TYPE_TEXT:
      ctxsys.oled.drawText(ev->getMsg1(), ev->getMsg2());
      break;

    case SystemEvent::TYPE_FUNC:
      ctxsys.oled.drawText(ev->execFunc1(), ev->execFunc2());
      break;

    default:
      {
      LOG_ERROR("unknown event type", ev->getType(), ev->getLife());
      }
      return F_RES_FAIL;
  }
  return F_RES_SUCCESS;
}

ActionResult F_EventHandler::childaction(unsigned long ms, FunctionCode_t fc) {
  if(fcode != F_NONE) { // expect F_CHG_OLED
    if(beginning) {
      chg_event = true;
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG("F_CHG_OLED", F_CHG_OLED, "right ? ", fcode);
      }
    }
#endif
    return F_RES_SUCCESS;
  }

  // check key and mcu error status
  // key error ignored when keyswCtrl::FORCE_OFF
  // keyerr and mcuerr is used for LED control. not used for event message generation.
  bool keyerr = false;
  char keyerr_msg[32];
  uint8_t keyerr_pos = 0;
  if(MagswStateMachine::getKeyErrorPresence()) {
    APPEND_STR(keyerr_msg, keyerr_pos, "KEY ");
    for(uint8_t i = 0; i < DEFS_NMAGKEYS; i++) {
      if(((MagswStateMachine *)CustomKeymap::ksm[i])->getErrorMaskedbyForceOff()) {
        keyerr = true;
        APPEND_STR(keyerr_msg, keyerr_pos, "%d ", i);
      }
    }
  }

  bool mcuerr = false;
  char mcuerr_msg[32];
  uint8_t mcuerr_pos = 0;
  APPEND_STR(mcuerr_msg, mcuerr_pos, "MCU ");
  for(uint8_t i = 0; i < MCU_NMCUS; i++) {
    if(ctxmcu.mcu[i].getError()) {
      mcuerr = true;
      APPEND_STR(mcuerr_msg, mcuerr_pos, "1");
    } else APPEND_STR(mcuerr_msg, mcuerr_pos, "0");
  }

  /*
  if(mcuerr || keyerr) { // push to error event Q
    current_event->setEvent(SystemEvent::TYPE_TEXT, 0, 0, keyerr ? keyerr_msg : "", mcuerr ? mcuerr_msg : "", nullptr, nullptr);
    event_type = EV_MCUKEYERR;
    sub_processed_time = ms;
  }
  */

  // set system status LED
  ctxsys.f_systemled.setSystemState(
    ctxsys.kb_ctrl.isCalibrationMode() || ctxsys.kb_ctrl.isPseudoSysFault(),
    ctxsys.kb_ctrl.isEmergencyMode(),
    mcuerr || ctxsys.kb_ctrl.getPseudoState(LEDTYPE_MCU),
    keyerr || ctxsys.kb_ctrl.getPseudoState(LEDTYPE_KEY));
  ctxsys.f_systemled.display(ms);

  // if new priority event exists or chg_event == true, change event immediately.
  if(!current_event || priority_q.getCount() > 0 || chg_event) {
    updateLifeOfCurrentEvent(true);
    // current_event == nulllptr. new event needed.
    current_event = priority_q.popFront();
    if(!current_event) {
      current_event = error_q.popFront();
      if(!current_event) {
        current_event = info_debug_q.popFront();
        // ignore debug event if not in debug mode or not DEBUG build
        if(current_event && current_event->getPriority() == SystemEvent::PR_DEBUG) {
#if defined(DEBUG)
          if(!ctxsys.kb_ctrl.isDebugMode()) 
#endif
          {
            current_event = info_debug_q.pushBack(current_event);
          }
        }
      }
    }

    if(current_event && current_event->getLife() == 0) {
      updateLifeOfCurrentEvent();
      return F_RES_NONE;
    }
    processed_time = sub_processed_time = ms; // reset timer for new event
    event_timer = (chg_event ? TIMER_LONG : TIMER_SHORT);

    drawOLED(current_event);
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
      LOG_DEBUG("chg_event: ", chg_event);
      dumpSystemQ();
    }
#endif

    /*
      if current_event is nullptr, search next event in next cycle.
    */
  }

  chg_event = false;
  if(!current_event) return F_RES_NONE;

  if(ms - sub_processed_time < sub_event_timer && ms - processed_time < event_timer) {
    return F_RES_SUCCESS;
  }
  sub_processed_time = ms;
  if(ms - processed_time < event_timer) {
    drawOLED(current_event);
    return F_RES_SUCCESS;
  }
  updateLifeOfCurrentEvent(true); // move to next event
  processed_time = sub_processed_time = ms;

  return F_RES_SUCCESS;
}


void F_SystemLED::begin() {
  // there is only one F_SystemLED instance.

  Adafruit_NeoPixel::begin();
  //setBrightness(keyled.led_brightness);
  Adafruit_NeoPixel::setBrightness(20);
  uint32_t color = Adafruit_NeoPixel::Color(255, 0, 0);

#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("System LED on");
    setPixelColor(LEDPOSITION_SYS, color);
#if defined(SUPPORT_LOCKLED)
    setPixelColor(LEDPOSITION_NUMLOCK, color);
    setPixelColor(LEDPOSITION_CAPSLOCK, color);
    setPixelColor(LEDPOSITION_SCROLLLOCK, color);
#endif
    show();
    delay(200);
  }
#endif

  setPixelColor(LEDPOSITION_SYS, 0);
#if defined(SUPPORT_LOCKLED)
  setPixelColor(LEDPOSITION_NUMLOCK, 0);
  setPixelColor(LEDPOSITION_CAPSLOCK, 0);
  setPixelColor(LEDPOSITION_SCROLLLOCK, 0);
#endif
  show();

  //KeySensorMCU::setLEDAttributesAll(keyled);
}

void F_SystemLED::enableBacklight(bool e) {
  keyled.backlight_enable = e;
  KeySensorMCU::setLEDAttributesAll(keyled);
}

ActionResult F_SystemLED::childaction(unsigned long ms, FunctionCode_t fc) {
  if(!beginning) { // not executed continuously, only on the first press.
    return F_RES_NONE;
  }

  if(fc == F_KLED_BACKLIGHT) {
    keyled.backlight_enable = !keyled.backlight_enable;
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Backlight enable ->", (int)keyled.backlight_enable);
    }
#endif
    ctxsys.ev_handler.pushNewEvent(
      SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1,
      "Back Light", (keyled.backlight_enable ? "Enabled" : "Disabled"));
    enableBacklight(keyled.backlight_enable);
  }
  
#define SYSTEMLED_BRIGHTNESS_STEP 25
  else  if(fc == F_BRIGHTUP) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("LED brightness UP ->", (int)keyled.led_brightness);
    }
#endif
    ctxsys.ev_handler.pushNewEvent(
      SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Brightness UP", nullptr);
    applyBrightness(
      keyled.led_brightness <= 255 - SYSTEMLED_BRIGHTNESS_STEP ?
        keyled.led_brightness + SYSTEMLED_BRIGHTNESS_STEP : 255);
  }
  
  else if(fc == F_BRIGHTDOWN) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("LED brightness DOWN ->", (int)keyled.led_brightness);
    }
#endif
    ctxsys.ev_handler.pushNewEvent(
      SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Brightness DOWN", nullptr);
    applyBrightness(
      keyled.led_brightness >= SYSTEMLED_BRIGHTNESS_STEP ?
        keyled.led_brightness - SYSTEMLED_BRIGHTNESS_STEP : 0);
  }

  else if(fc == F_BRIGHTNESS) {
    uint8_t v = getInputValue32() & 0xff;
    switch(v) {
      case 1:
        applyBrightness(250);
        enableBacklight(false);
        ctxsys.ev_handler.pushNewEvent(
          SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Auto Bright 250", "Backlight Off");
        break;
      case 2:
        applyBrightness(150);
        enableBacklight(false);
        ctxsys.ev_handler.pushNewEvent(
          SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Auto Bright 150", "Backlight Off");
        break;
      case 3:
        applyBrightness(70);
        enableBacklight(false);
        ctxsys.ev_handler.pushNewEvent(
          SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Auto Bright 70", "Backlight Off");
        break;
      case 4:
        applyBrightness(30);
        enableBacklight(false);
        ctxsys.ev_handler.pushNewEvent(
          SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Auto Bright 30", "Backlight Off");
        break;
      case 5:
        applyBrightness(30);
        enableBacklight(true);
        ctxsys.ev_handler.pushNewEvent(
          SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Auto Bright 30", "Backlight On");
        break;
    }

#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("LED brightness v", v, ",", (int)keyled.led_brightness);
    }
#endif
  }

  else {
    LOG_ERROR("unknown function code", fc);
    return F_RES_FAIL;
  }

  return F_RES_SUCCESS;
};

void F_SystemLED::display(unsigned long ms) {
#if defined(SUPPORT_LOCKLED)
  uint32_t led_color[4];
#else
  uint32_t led_color[1];
#endif

  if(processed_time > ms) {
    processed_time = ms;
  }

  if(type_state[LEDTYPE_CALIBRATION]) {
    if(ms - processed_time > 1000) {
      sysblink = !sysblink;
      processed_time = ms;
    }

    led_color[LEDPOSITION_SYS] = (sysblink ? kbled.color[LEDTYPE_CALIBRATION] : 0);
#if defined(SUPPORT_LOCKLED)
    led_color[LEDPOSITION_NUMLOCK] = 0;
    led_color[LEDPOSITION_CAPSLOCK] = 0;
    led_color[LEDPOSITION_SCROLLLOCK] = 0;
#endif

  } else {
    // search type in non-zero state
    // if not found, type_active stays on same position.
    uint8_t index = type_active;
    bool found = false;
    for(uint8_t i = 0; i < LEDTYPE_SYSMAX; i++) {
      if(type_state[index]) {
        found = true;
        break;
      }
      if(++index >= LEDTYPE_SYSMAX) {
        index = 0;
      }
    }

    if(found) {
      if(index != type_active) { // new
        processed_time = ms;
        type_active = index;
      } else if(ms - processed_time > 500) { // timeout, increment head position and return
        processed_time = ms;
        if(++type_active >= LEDTYPE_SYSMAX) {
          type_active = 0;
        }
      }
    }

    led_color[LEDPOSITION_SYS] = (type_state[type_active] ? kbled.color[type_active] : 0);
#if defined(SUPPORT_LOCKLED)
    led_color[LEDPOSITION_NUMLOCK] = (type_state[LEDTYPE_NUMLOCK] ? kbled.color[LEDTYPE_NUMLOCK] : 0);
    led_color[LEDPOSITION_CAPSLOCK] = (type_state[LEDTYPE_CAPSLOCK] ? kbled.color[LEDTYPE_CAPSLOCK] : 0);
    led_color[LEDPOSITION_SCROLLLOCK] = (type_state[LEDTYPE_SCROLLLOCK] ? kbled.color[LEDTYPE_SCROLLLOCK] : 0);
#endif
  }

  if(memcmp(led_color, led_color_prev, sizeof(led_color)) != 0) {
    setPixelColor(LEDPOSITION_SYS, led_color[LEDPOSITION_SYS]);
#if defined(SUPPORT_LOCKLED)
    setPixelColor(LEDPOSITION_NUMLOCK, led_color[LEDPOSITION_NUMLOCK]);
    setPixelColor(LEDPOSITION_CAPSLOCK, led_color[LEDPOSITION_CAPSLOCK]);
    setPixelColor(LEDPOSITION_SCROLLLOCK, led_color[LEDPOSITION_SCROLLLOCK]);
#endif
    show();

    memcpy(led_color_prev, led_color, sizeof(led_color));

    /*
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
#if defined(SUPPORT_LOCKLED)
      LOG_DEBUG("diff", ms - processed_time, ", active", type_active, ", LED Color", DebugLogBase::HEX, led_color[LEDPOSITION_SYS], led_color[LEDPOSITION_NUMLOCK], led_color[LEDPOSITION_CAPSLOCK], led_color[LEDPOSITION_SCROLLLOCK]);
#else
      LOG_DEBUG("diff", ms - processed_time, ", active", type_active, ", LED Color", DebugLogBase::HEX, led_color[LEDPOSITION_SYS]);
#endif
    }
#endif
*/
  }
}


// for sysled, webhid
void F_SystemLED::setLEDColor(uint8_t type, uint8_t r, uint8_t g, uint8_t b) {
  if(type >= LEDTYPE_MAX) {
    LOG_ERROR("unknown type", type);
    return;
  }
  kbled.color[type] = Color(r, g, b) | (kbled.color[type] & 0xFF000000);
}

/*
void F_SystemLED::setKeyLEDColor(uint8_t r, uint8_t pp, uint8_t p, uint8_t alt, uint8_t backlight) {
  I2CLEDAttributes kl;
  kl.led_ptn_release = r;
  kl.led_ptn_partialpress = pp;
  kl.led_ptn_press = p;
  kl.led_ptn_alert = alt;
  kl.led_backlight = backlight;
  kl.led_brightness = keyled.led_brightness;
  if(validateLEDAttr(kl)) {
    keyled.led_ptn_release = r;
    keyled.led_ptn_partialpress = pp;
    keyled.led_ptn_press = p;
    keyled.led_ptn_alert = alt;
    keyled.led_backlight = backlight;
    KeySensorMCU::setLEDAttributesAll(keyled);
  }
}
*/

// for keyled, webhid
void F_SystemLED::setKeyLEDColor(uint8_t type, uint8_t ptn) {
  switch(type) {
    case KEYLED_RELEASE:
      keyled.led_ptn_release = ptn;
      break;
    case KEYLED_PARTIALPRESS:
      keyled.led_ptn_partialpress = ptn;
      break;
    case KEYLED_PRESS:
      keyled.led_ptn_press = ptn;
      break;
    case KEYLED_ALERT:
      keyled.led_ptn_alert = ptn;
      break;
    default:
      LOG_ERROR("unknown key LED type", type);
      return;
  }

  validateLEDAttr(keyled);
  KeySensorMCU::setLEDAttributesAll(keyled);
}

// for sysled, webhid
void F_SystemLED::applyBrightness(uint8_t brightness) {
  if(brightness < 5) {
    brightness = 5;
  }

  keyled.led_brightness = brightness;

#if defined(SUPPORT_SYS_RGB_REDUCE_BRIGHTNESS)
    Adafruit_NeoPixel::setBrightness(brightness / 3);
#else
    Adafruit_NeoPixel::setBrightness(brightness);
#endif

  KeySensorMCU::setLEDAttributesAll(keyled);
}

void F_SystemLED::applyBacklight(uint8_t backlight) {
  keyled.led_backlight = backlight;
  KeySensorMCU::setLEDAttributesAll(keyled);
}

// for webhid
uint8_t F_SystemLED::getLEDColorR(ConfigKeyboardLED &ckled, uint8_t type) {
  return(type < LEDTYPE_MAX ? (ckled.color[type] >> 16) & 0xff : 0);
}
uint8_t F_SystemLED::getLEDColorG(ConfigKeyboardLED &ckled, uint8_t type) {
  return(type < LEDTYPE_MAX ? (ckled.color[type] >> 8) & 0xff : 0);
}
uint8_t F_SystemLED::getLEDColorB(ConfigKeyboardLED &ckled, uint8_t type) {
  return(type < LEDTYPE_MAX ? (ckled.color[type]) & 0xff : 0);
}

bool F_SystemLED::validateLEDAttr(I2CLEDAttributes &ledattr) {
  bool r = true;

  if(ledattr.led_ptn_release >= I2CCtrl::LED_PTN_MAX) {
    LOG_ERROR("Invalid LED pattern for release: ", ledattr.led_ptn_release);
    ledattr.led_ptn_release = LEDConst::LED_PTN_RELEASE;
    r = false;
  }
  if(ledattr.led_ptn_partialpress >= I2CCtrl::LED_PTN_MAX) {
    LOG_ERROR("Invalid LED pattern for partial press: ", ledattr.led_ptn_partialpress);
    ledattr.led_ptn_partialpress = LEDConst::LED_PTN_PARTIALPRESS;
    r = false;
  }
  if(ledattr.led_ptn_press >= I2CCtrl::LED_PTN_MAX) {
    LOG_ERROR("Invalid LED pattern for press: ", ledattr.led_ptn_press);
    ledattr.led_ptn_press = LEDConst::LED_PTN_PRESS;
    r = false;
  }
  if(ledattr.led_ptn_alert >= I2CCtrl::LED_PTN_MAX) {
    LOG_ERROR("Invalid LED pattern for alert: ", ledattr.led_ptn_alert);
    ledattr.led_ptn_alert = LEDConst::LED_PTN_ALERT;
    r = false;
  }
  // backlight can be between 0 - 255, no validation

  return r;
}


ActionResult F_KeyboardReset::childaction(unsigned long ms, FunctionCode_t fc) {
  switch(fcode) {
    case F_KBD_RESET:
      F_Key::clearHidReport();
      F_Mouse::clearHidReport();
      rp2040.reboot();
      break;

    case F_MCU_RESET:
      {
        delay(1000);
        multicoremsg_t msg;
        msg.cmd = I2CCtrl::MCURESET;
        multicoreMsgPush(msg);
        multicoreMsgPop(msg);
      }
      break;

    default:
      break;
  }

  return F_RES_SUCCESS;
}

void OLED::begin(uint8_t addr) {
  Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC, addr, false, false);
  setTextColor(SSD1306_WHITE);
  setTextSize(1);
  cp437(true);
  // just begin, do not display
}

void OLED::drawText(const char *t1, const char *t2) {
  // clear display if !t1 and !t2

  clearDisplay();

  bool line1 = true, line2 = true;
  if(t1 == nullptr) {
    line1 = false;
  }
  if(t2 == nullptr) {
    line2 = false;
  }

  if(line1 && line2) { // draw both lines
    setFont(&unifont_17_0_0316pt7b);
    setCursor(0, 12);
    print(t1);
    setCursor(0, 26);
    print(t2);
  } else if(line1) { // draw single line
    setFont(&spleen_12x2424pt7b);
    setCursor(0, 24);
    print(t1);
  } else if(line2) {
    setFont(&spleen_12x2424pt7b);
    setCursor(0, 24);
    print(t2);
  }

  multicoremsg_t msg;
  msg.cmd = I2CCtrl::OLED;
  msg.oled_cmd = CMD_DISPLAY;
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
}

void KeyboardControl::setDebugModeAll() {
  multicoremsg_t msg;
  msg.cmd = I2CCtrl::SET_DEBUG;
  msg.debug_mode = (debug ? 1 : 0);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
}

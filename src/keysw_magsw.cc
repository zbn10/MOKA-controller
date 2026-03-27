#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "keysw_statemachine.h"


appcontext_magsw_t &appctxMagsw() {
  static KeyswAttributes magsw_attr_list[DEFS_NMAGKEYS];
  static MagswStateMachine msm[DEFS_NMAGKEYS] = {
    MagswStateMachine(magsw_attr_list[0]),
    MagswStateMachine(magsw_attr_list[1]),
    MagswStateMachine(magsw_attr_list[2]),
    MagswStateMachine(magsw_attr_list[3]),
    MagswStateMachine(magsw_attr_list[4]),
    MagswStateMachine(magsw_attr_list[5]),
    MagswStateMachine(magsw_attr_list[6]),
    MagswStateMachine(magsw_attr_list[7]),
    MagswStateMachine(magsw_attr_list[8]),
    MagswStateMachine(magsw_attr_list[9]),
    MagswStateMachine(magsw_attr_list[10]),
    MagswStateMachine(magsw_attr_list[11]),
    MagswStateMachine(magsw_attr_list[12]),
    MagswStateMachine(magsw_attr_list[13]),
    MagswStateMachine(magsw_attr_list[14]),
    MagswStateMachine(magsw_attr_list[15]),
    MagswStateMachine(magsw_attr_list[16]),
    MagswStateMachine(magsw_attr_list[17]),
    MagswStateMachine(magsw_attr_list[18]),
    MagswStateMachine(magsw_attr_list[19]),
    MagswStateMachine(magsw_attr_list[20]),
    MagswStateMachine(magsw_attr_list[21]),
    MagswStateMachine(magsw_attr_list[22]),
    MagswStateMachine(magsw_attr_list[23]),
    MagswStateMachine(magsw_attr_list[24]),
    MagswStateMachine(magsw_attr_list[25]),
    MagswStateMachine(magsw_attr_list[26]),
    MagswStateMachine(magsw_attr_list[27]),
    MagswStateMachine(magsw_attr_list[28]),
    MagswStateMachine(magsw_attr_list[29]),
    MagswStateMachine(magsw_attr_list[30]),
    MagswStateMachine(magsw_attr_list[31]),
    MagswStateMachine(magsw_attr_list[32]),
    MagswStateMachine(magsw_attr_list[33]),
    MagswStateMachine(magsw_attr_list[34]),
    MagswStateMachine(magsw_attr_list[35]),
    MagswStateMachine(magsw_attr_list[36]),
    MagswStateMachine(magsw_attr_list[37]),
    MagswStateMachine(magsw_attr_list[38]),
    MagswStateMachine(magsw_attr_list[39]),
    MagswStateMachine(magsw_attr_list[40]),
    MagswStateMachine(magsw_attr_list[41]),
  };
  static appcontext_magsw_t ctx = {
    .magsw_attr_list = magsw_attr_list,
    .msm = msm,
  };
  return ctx;
}

bool MagswStateMachine::isRelease2PartialPress() {
  if(keyval >= keyattr.thresh_partial_press && keyval < keyattr.thresh_press) {
    if(!partial_pressing) {
      partial_pressing = true;
      millis_partial_pressing = now;
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG("PartialPressing: keyval", keyval, ">=", keyattr.thresh_partial_press, "& <", keyattr.thresh_press, "& >", keyattr.duration_partial_press);
      }
#endif
      return(false);
    }

    unsigned long diff = now - millis_partial_pressing;
    if(diff >= keyattr.duration_partial_press) {
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG("PartialPress: diff", diff, ">=", keyattr.duration_partial_press);
      }
#endif
      partial_pressing = false;
      return(true);
    }

    return(false);
  }

  partial_pressing = false;
  return(false);
}

bool MagswStateMachine::isPartialPress2Releasing() {
  if(keyval < keyattr.thresh_partial2release) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, "< thresh_partial2release", keyattr.thresh_partial2release);
    }
#endif
    return(true);
  }
  return(false);
}

// for tap-advanced
bool MagswStateMachine::isRelease2Press() { 
  if(keyval >= keyattr.thresh_press) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, ">= thresh_press", keyattr.thresh_press);
    }
#endif
    millis_press = now; // for checking of duration in isPress2Tap(), etc.
    return(true);
  }
  return(false);
}
// for tap-advanced
bool MagswStateMachine::isPress2Tap() { // duration check is done in checkTransition()
  unsigned long diff = now - millis_press;

  if(keyval < keyattr.thresh_press2release && diff < keyattr.duration_hold) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, "< thresh_p2r", keyattr.thresh_press2release, ", duration diff", diff, "< keyattr.duration_hold", keyattr.duration_hold);
    }
#endif
    return(true);
  }
  return(false);
}

// for tap-normal
bool MagswStateMachine::isRelease2Pressing() {
  if(keyval >= keyattr.thresh_press) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, ">= thresh_press", keyattr.thresh_press);
    }
#endif
    millis_press = now; // for checking of duration in isPressing2Tap(), isPressing2Press(), etc.
    return(true);
  }
  return(false);
}
// for tap-normal
bool MagswStateMachine::isPressing2Tap() {
  unsigned long diff = now - millis_press;

  if(keyval < keyattr.thresh_press2release && diff < keyattr.duration_hold) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, "< thresh_p2r", keyattr.thresh_press2release, ", duration diff", diff, "< keyattr.duration_hold", keyattr.duration_hold);
    }
#endif
    return(true);
  }
  return(false);
}
// for tap-normal
bool MagswStateMachine::isPressing2Press() {
  unsigned long diff = now - millis_press;

  if(keyval >= keyattr.thresh_press2release && diff >= keyattr.duration_hold) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, ">= thresh_p2r", keyattr.thresh_press2release, ", duration diff", diff, ">= keyattr.duration_hold", keyattr.duration_hold);
    }
#endif
    return(true);
  }
  return(false);
}

bool MagswStateMachine::isPress2Hold() { // duration check is done in checkTransition()
  unsigned long diff = now - millis_press;

  if(keyval >= keyattr.thresh_press && diff >= keyattr.duration_hold) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, ">= thresh_press", keyattr.thresh_press, ", duration diff", diff, ">= keyattr.duration_hold", keyattr.duration_hold);
    }
#endif
    return(true);
  }
  return(false);
}

bool MagswStateMachine::isPress2HoldL() {
  unsigned long diff = now - millis_press;

  if(keyval >= keyattr.thresh_press && diff >= keyattr.duration_holdL) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, ">= thresh_press", keyattr.thresh_press, ", duration diff", diff, ">= keyattr.duration_holdL", keyattr.duration_holdL);
    }
#endif
    return(true);
  }
  return(false);
}

bool MagswStateMachine::isPress2HoldLL() {
  unsigned long diff = now - millis_press;

  if(keyval >= keyattr.thresh_press && diff >= keyattr.duration_holdLL) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval", keyval, ">= thresh_press", keyattr.thresh_press, ", duration diff", diff, ">= keyattr.duration_holdLL", keyattr.duration_holdLL);
    }
#endif
    return(true);
  }
  return(false);
}

bool MagswStateMachine::isPress2Releasing() {
  if(keyval < keyattr.thresh_press2release) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval ", keyval, "< thresh_press2release ", keyattr.thresh_press2release);
    }
#endif
    return(true);
  }
  return(false);
}

bool MagswStateMachine::isReleasing2Release() {
  if(keyval < keyattr.thresh_partial2release) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("keyval ", keyval, "< thresh_partial2release ", keyattr.thresh_partial2release);
    }
#endif
    millis_partial = millis_press = 0;
    return(true);
  }
  return(false);
}

bool MagswStateMachine::checkTransition(bool tapadv) {
  KeyswState state_int_prev = state_internal;
  state_prev = state;
  physical_state_prev = physical_state;
  now = millis();

/*
  Release -> PartialPress
  PartialPress -> keep
  PartialPress -> Press
  PartialPress -> Releasing

  Press -> keep
  Press -> Releasing
  Press -> Hold

  Hold -> keep
  Hold -> Releasing
  Hold -> HoldL

  HoldL -> keep
  HoldL -> Releasing
  HoldL -> HoldLL

  HoldLL -> keep
  HoldLL -> Releasing

  Releasing -> Release
*/

  if(getErrorOrForceOff()) {
    state = state_internal = KS_RELEASE;
    physical_state = KS_RELEASE;

    if(getError()) {
      if(!err_logged) {
        LOG_ERROR("Key error detected");
        ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Key Error", "detected");
      }
      err_logged = true;
    }
    return(false);
  }
  err_logged = false;

  switch(state_internal) {
    case KS_RELEASE:
      if(isRelease2PartialPress()) {
        state = state_internal = KS_PARTIAL;
        physical_state = KS_PARTIAL;
      } else if(tapadv) {
        if(isRelease2Press()) {
          state = state_internal = KS_PRESS;
          physical_state = KS_PRESS;
        }
      } else {
        if(isRelease2Pressing()) {
          state = KS_RELEASE;
          state_internal = KS_PRESSING;
          physical_state = KS_RELEASE;
        }
      }
      break;

    case KS_PRESSING: // used in tap-normal only
      if(isPressing2Tap()) {
        state = state_internal = KS_TAP;
        physical_state = KS_PRESS;
      } else if(isPressing2Press()) {
        state = state_internal = KS_PRESS;
        physical_state = KS_PRESS;
      }
      break;

    case KS_PARTIAL:
      if(isRelease2Press()) { // take care of tap-advanced behavior
        state = state_internal = KS_PRESS;
        physical_state = KS_PRESS;
      } else if(isPartialPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      }
      break;

    case KS_PRESS:
      if(tapadv) {
        if(isPress2Tap()) {
          state = state_internal = KS_TAP;
          physical_state = KS_PRESS;
          break;
        }
      }

      if(isPress2Hold()) {
        state = state_internal = KS_HOLD;
        physical_state = KS_PRESS;
      } else if(isPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      }
      break;

    case KS_TAP:
      if(!tapping) {
        tapping = true;
        millis_tap = now;
      } else if(now - millis_tap > 20) {
        tapping = false;
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      }
      break;

    case KS_HOLD:
      if(isPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      } else if(isPress2HoldL()) {
        state = state_internal = KS_HOLDL;
        physical_state = KS_PRESS;
      }
      break;

    case KS_HOLDL:
      if(isPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      } else if(isPress2HoldLL()) {
        state = state_internal = KS_HOLDLL;
        physical_state = KS_PRESS;
      }
      break;

    case KS_HOLDLL:
      if(isPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      }

      break;

    case KS_RELEASING:
      if(isReleasing2Release()) {
        state = state_internal = KS_RELEASE;
        physical_state = KS_RELEASE;
      }
      break;
  }

  bool b = (state != state_prev);
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode() && state_int_prev != state_internal) {
    LOG_DEBUG(getStateName(state_int_prev), "->", getStateName(state_internal));
  }
#endif

  return(b);
}


uint8_t MagswStateMachine::setKeyval(uint8_t kv) {
  switch(keyattr.control) {
    case KC_FORCE_OFF:
      keyval = 0;
      break;

    case KC_FORCE_ON:
      keyval = 255;
      break;

    default:
      keyval = kv;
      break;
  }

  return(keyval);
}

bool MagswStateMachine::setKeyattr(uint8_t tpp, uint8_t tp, uint8_t tpp2r, uint8_t tp2r, uint16_t dur_hold) {
  if(!validateKeyattr(tpp, tp, tpp2r, tp2r, dur_hold)) return false;
  keyattr.thresh_partial_press = tpp;
  keyattr.thresh_press = tp;
  keyattr.thresh_partial2release = tpp2r;
  keyattr.thresh_press2release = tp2r;
  keyattr.duration_hold = dur_hold;
  return true;
}

bool MagswStateMachine::validateKeyattr(uint8_t tpp, uint8_t tp, uint8_t tpp2r, uint8_t tp2r, uint16_t dur_hold) {
  if(tpp <= tpp2r) {
    LOG_ERROR("Invalid key thresholds: Partial Press", tpp, "<= Partial Release", tpp2r);
    return false;
  }
  if(tp <= tpp) {
    LOG_ERROR("Invalid key thresholds: Press", tp, "<= Partial Press", tpp);
    return false;
  }
  if(tp <= tp2r) {
    LOG_ERROR("Invalid key thresholds: Press", tp, "<= Release", tp2r);
    return false;
  }
  if(dur_hold >= KeyswConst::DurHoldL) {
    LOG_ERROR("Invalid hold duration", dur_hold, ">= HoldL", KeyswConst::DurHoldL);
    return false;
  }
  return true;
}

bool MagswStateMachine::validateKeyattr(KeyswAttributes (&keyattr)[DEFS_NMAGKEYS]) {
  bool r = true;
  for(int key = 0; key < DEFS_NMAGKEYS; key++) {
    if(!validateKeyattr(keyattr[key].thresh_partial_press,
                        keyattr[key].thresh_press,
                        keyattr[key].thresh_partial2release,
                        keyattr[key].thresh_press2release,
                        keyattr[key].duration_hold)) {
      LOG_ERROR("Invalid keyattr for key ", key,
        ", tpp ", keyattr[key].thresh_partial_press, ", tp ", keyattr[key].thresh_press,
        ", tpp2r ", keyattr[key].thresh_partial2release, ", tp2r ", keyattr[key].thresh_press2release,
        ", dur_hold ", keyattr[key].duration_hold);
      keyattr[key].thresh_partial_press = KeyswConst::ThrPartialPress;
      keyattr[key].thresh_press = KeyswConst::ThrPress;
      keyattr[key].thresh_partial2release = KeyswConst::ThrPartial2Release;
      keyattr[key].thresh_press2release = KeyswConst::ThrPress2Release;
      keyattr[key].duration_hold = KeyswConst::DurHold;
      r = false;
    }
    if(keyattr[key].control >= KC_MAX) {
      LOG_ERROR("Invalid key control for key ", key, ", ctrl ", keyattr[key].control);
      keyattr[key].control = KC_NONE;
      r = false;
    }
  }

  return r;
}

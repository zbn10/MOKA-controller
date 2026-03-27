#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "keysw_statemachine.h"
#include "i2c_control.h"

appcontext_ksm_t &appctxKsm() {
  static TactswStateMachine tact_sw(DEFS_PIN_FUNCBTN);
#if defined(SUPPORT_TINYMOUSE)
  static TinyMouseStateMachine tmouse(DEFS_PIN_JOYSTICK_X, DEFS_PIN_JOYSTICK_Y);
#endif
#if defined(SUPPORT_TOUCHPAD)
  static TouchpadStateMachine tpsm_left(I2CCtrl::TPL_ADDR);
  static TouchpadStateMachine tpsm_right(I2CCtrl::TPR_ADDR);
#endif
#if defined(SUPPORT_PHOTOIC_INFRARED)
  static PhotoICSensorStateMachine photoic_sw(DEFS_PIN_PHOTOIC);
  static InfraredSensorStateMachine infrared_sw(DEFS_PIN_INFRARED);
#endif
  static PseudokeyStateMachine dummy_sw;

  static appcontext_ksm_t ctx = {
    tact_sw,
#if defined(SUPPORT_TINYMOUSE)
    tmouse,
#endif
#if defined(SUPPORT_TOUCHPAD)
    tpsm_left,
    tpsm_right,
#endif
#if defined(SUPPORT_PHOTOIC_INFRARED)
    photoic_sw,
    infrared_sw,
#endif
    dummy_sw,
  };
  return ctx;
}



const char *KeyswStateMachine::getActiveStateName() {
  if(key_err) return("KeyError");
  return(getStateName(state));
}

const char *KeyswStateMachine::getStateName(KeyswState s) {
  switch(s) {
    case KS_RELEASE:
      return("Release");
    case KS_PARTIAL:
      return("Partial");
    case KS_PRESSING:
      return("Pressing");
    case KS_PRESS:
      return("Press");
    case KS_TAP:
      return("Tap");
    case KS_HOLD:
      return("Hold");
    case KS_HOLDL:
      return("HoldLong");
    case KS_HOLDLL:   
      return("LHoldLong2");
    case KS_RELEASING:
      return("Releasing");
    case KS_VALUE:
      return("Value");
  }
  return "Unknown";
}

bool TactswStateMachine::swPressing() {
  bool b = (digitalRead(pin) == LOW);
  keyval = (b ? 255 : 0);
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("digitalRead", b);
  }
#endif
  return(b);
}

bool TactswStateMachine::isRelease2Press() {
  if(swPressing()) {
    if(!pressing) {
      pressing = true;
      millis_pressing = now;
      return(false);
    }

    if(now - millis_pressing > DurPressing) {
      pressing = false;
      return(true);
    } else return(false);
  }

  pressing = false;
  return(false);
}

bool TactswStateMachine::isPress2HoldL() {
  if(swPressing() && now - millis_press > KeyswConst::DurHoldL) {
    return(true);
  }
  return(false);
}

bool TactswStateMachine::isPress2HoldLL() {
  if(swPressing() && now - millis_press > KeyswConst::DurHoldLL) {
    return(true);
  }
  return(false);
}

bool TactswStateMachine::isPress2Releasing() {
  if(!swPressing()) {
    if(!releasing) {
      releasing = true;
      millis_releasing = now;
      return(false);
    }

    if(now - millis_releasing > DurPressing) { // DurPressing means DurReleasing here
      releasing = false;
      return(true);
    } else return(false);

  }

  releasing = false;
  return(false);
}

bool TactswStateMachine::checkTransition(bool tapadv) {
  now = millis();
  KeyswState state_internal_prev = state_internal;
  state_prev = state;
  physical_state_prev = physical_state;

  // reset state if loop() stuck for long time
  if(now - lasttime > 1000) {
    state = state_internal = KS_RELEASE;
    physical_state = KS_RELEASE;
  }

  switch(state_internal) {
    case KS_RELEASE:
      if(isRelease2Press()) {
        millis_press = now;
        state = state_internal = KS_PRESS;
        physical_state = KS_PRESS;
      }
      break;

    case KS_PRESS:
      if(isPress2HoldL()) {
        state = state_internal = KS_HOLDL;
        physical_state = KS_PRESS;
      } else if(isPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
      }
      break;

    case KS_HOLDL:
      if(isPress2HoldLL()) {
        state = state_internal = KS_HOLDLL;
        physical_state = KS_PRESS;
      } else if(isPress2Releasing()) {
        state = KS_RELEASE;
        state_internal = KS_RELEASING;
        physical_state = KS_RELEASE;
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

    default:
      LOG_ERROR("Invalid state: ", state_internal);
      state = state_internal = KS_RELEASE;
      physical_state = KS_RELEASE;
      break;
  }

#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode() && state_internal_prev != state_internal) {
    LOG_DEBUG("state", getStateName(state_internal_prev), "->", getStateName(state_internal));
  }
#endif

  lasttime = now;
  return(state_prev != state);
}

bool PhotoICSensorStateMachine::checkTransition(bool tapadv) {
  unsigned long now = millis();
  physical_state_prev = state_prev = state;
  if(now - last_transition_time < 100) {
    physical_state = state = KS_RELEASE;
    return false;
  }

  last_transition_time = now;
  int32_t v = analogRead(pin);
  value += (v - value) / 30; // average for 3 seconds

  if(now - last_state_changed_time < 3000) {
    physical_state = state = KS_RELEASE;
    return false;
  }

  last_state_changed_time = now;
  anyvalue = (value > 2000 ? 1 : (value > 1500 ? 2 : (value > 1000 ? 3 : (value > 500 ? 4 : 5))));
  if(anyvalue == anyvalue_prev) {
    physical_state = state = KS_RELEASE;
    return false;
  }

  physical_state = state = KS_VALUE;
  anyvalue_prev = anyvalue;
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("value", value, ", analogRead()", v, ", anyvalue", anyvalue);
  }
#endif
  return true;
}


bool InfraredSensorStateMachine::checkTransition(bool tapadv) {
  unsigned long millis_now = millis();
  state_prev = state;
  physical_state_prev = physical_state;
/*
  bool p = digitalRead(pin);
  if(p) {
    millis_lastexists = millis_now;
  }
*/
  value = analogRead(pin);
  if(value > (1<<(DEFS_ANALOGRESOLUTION - 3))) {
    millis_lastexists = millis_now;
    anyvalue = 0;
  }

  unsigned long diff = millis_now - millis_lastexists;
  if(diff > 600000) { // 10 minutes
    state = KS_VALUE;
    physical_state = KS_VALUE;
    anyvalue = 1;
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
      LOG_DEBUG("analogRead() = ", value, ", anyvalue", anyvalue);
    }
#endif
  }
  else {
    state = KS_RELEASE;
    physical_state = KS_RELEASE;
  }

  return(state == KS_VALUE);
}

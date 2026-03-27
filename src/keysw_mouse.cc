#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "keysw_statemachine.h"
#include "functions.h"


bool TinyMouseStateMachine::checkAvailable() {
  available = (analogRead(x_pin) > 10 && analogRead(y_pin) > 10);
  return available;
}

void TinyMouseStateMachine::calibration() {
  int16_t x, y;
  x = analogRead(x_pin);
  y = analogRead(y_pin);
  delay(10);
  x += analogRead(x_pin);
  y += analogRead(y_pin);
  delay(10);
  x = (x + analogRead(x_pin)) / 3;
  y = (y + analogRead(y_pin)) / 3;

  X.center = x;
  Y.center = y;
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("x_center:", X.center, ", y_center:", Y.center);
  }
#endif
}

bool TinyMouseStateMachine::checkTransitionAxis(struct params &axis, unsigned long now) {
  bool moved = true;

  if(axis.delta > -50 && axis.delta < 50) {
    if(!axis.centered) {
      axis.centered = true;
      axis.micros_centered = now;
    } else if(now - axis.micros_centered > 10000) {
      moved = false;
    }
  }
  else {
    axis.centered = false;
  }

  if(moved) {
    unsigned long diff = now - axis.micros_moved;
    if(((axis.delta < -400 || axis.delta > 400) && diff > 10000)
      || ((axis.delta < -300 || axis.delta > 300) && diff > 15000)
      || ((axis.delta < -200 || axis.delta > 200) && diff > 20000)
      || (diff > 30000)) {
      axis.micros_moved = now;
    } else {
      moved = false;
    }
  }

  return(moved);
}

bool TinyMouseStateMachine::checkTransition(bool tapadv) {
  int16_t x = analogRead(x_pin);
  int16_t y = analogRead(y_pin);
  X.delta += ((x - X.center) - X.delta) / 5; // int16_t, average of 5 measurements
  Y.delta += ((y - Y.center) - Y.delta) / 5; // int16_t, average of 5 measurements
  unsigned long micros_now = micros();

  bool x_moved = checkTransitionAxis(X, micros_now);
  bool y_moved = checkTransitionAxis(Y, micros_now);

  F_Mouse::Movement mv;
  mv.x = (x_moved ? (X.delta > 0 ? 16 : -16) : 0);
  mv.y = (y_moved ? (Y.delta > 0 ? 16 : -16) : 0);
  anyvalue = mv.value32;
  state_prev = state;
  state = (x_moved || y_moved ? KS_VALUE : KS_RELEASE);

#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("x_pln/ctr/dlt:", x, X.center, X.delta, ", y_pln/ctr/dlt:", y, Y.center, Y.delta,
      ", state:", getStateName(state), ", anyvalue:0x", DebugLogBase::HEX, anyvalue);
  }
#endif

  return(state != state_prev);
}

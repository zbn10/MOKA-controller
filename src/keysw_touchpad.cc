#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "keysw_statemachine.h"
#include "functions.h"


bool TouchpadStateMachine::getRawval() {
  if(!isAvailable()) return false;

  multicoremsg_t msg;

  msg.cmd = I2CCtrl::GET_TPRAWVAL;
  msg.addr = getAddr();
  msg.buf = (uint8_t *)&i2cdata_rawval;
  msg.bufsize = sizeof(i2cdata_rawval);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  return(msg.result == 0);
}

bool TouchpadStateMachine::getParam() {
  if(!isAvailable()) return false;

  multicoremsg_t msg;

  uint32_t r;
  msg.cmd = I2CCtrl::GET_TPPARAM;
  msg.addr = getAddr();
  msg.buf = (uint8_t *)&i2cdata_param;
  msg.bufsize = sizeof(i2cdata_param);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  return(msg.result == 0);
}

bool TouchpadStateMachine::setParam(uint8_t gain, uint8_t prescale, uint8_t oversamples, uint8_t rsel, uint16_t dzx, uint16_t dzy, bool reset) {
  if(!isAvailable()) return false;

  multicoremsg_t msg;

  if(gain > 5) {
    LOG_ERROR("Invalid gain:", gain, "> 5");
    return false;
  }
  i2cdata_param.gain = gain;

  if(prescale > 7) {
    LOG_ERROR("Invalid prescale:", prescale, "> 7");
    return false;
  }
  i2cdata_param.prescale = prescale;

  if(oversamples > 6) {
    LOG_ERROR("Invalid oversamples:", oversamples, "> 6");
    return false;
  }
  i2cdata_param.oversamples = oversamples;

  if(rsel > 5) {
    LOG_ERROR("Invalid rsel:", rsel, "> 5");
    return false;
  }
  i2cdata_param.rsel = rsel;
  i2cdata_param.deadzone_x = dzx;
  i2cdata_param.deadzone_y = dzy;

  if(reset) {
    msg.cmd = I2CCtrl::RESET;
    msg.addr = getAddr();
    multicoreMsgPush(msg);
    multicoreMsgPop(msg);
    delay(200);
  }

  msg.cmd = I2CCtrl::SET_TPPARAM;
  msg.addr = getAddr();
  msg.buf = (uint8_t *)&i2cdata_param;
  msg.bufsize = sizeof(i2cdata_param);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  return(msg.result == 0);
}

int TouchpadStateMachine::showVal(Stream *s) {
  if(!isAvailable()) return -1;
  //s->print("TouchpadStateMachine values\n");

  s->print("touch:");
  s->print((int)(tpdata.touch & I2CTPData_TOUCH));
  s->print(", deltamax:");
  s->print((int)I2CTPData_GET_DELTAMAX(tpdata.touch));
  s->print(", x:");
  s->print((int)tpdata.x);
  s->print(", y:");
  s->print((int)tpdata.y);
  s->print('\n');

  if(!getRawval()) {
    LOG_ERROR("TouchpadStateMachine getRawval failed");
    return -1;
  }
  s->print("rawval\n  x:");
  s->print((int)i2cdata_rawval.x);
  s->print(", y:");
  s->print((int)i2cdata_rawval.y);
  s->print('\n');
  for(uint8_t y = 0; y < 5; y++) {
    for(uint8_t x = 0; x < 5; x++) {
      s->print('\t');
      s->print(i2cdata_rawval.touch[y] & 1);
      s->print('/');
      s->print(i2cdata_rawval.v[y][x]);
      i2cdata_rawval.touch[y] >>= 1;
    }
    s->print('\n');
  }

  if(!getParam()) {
    LOG_ERROR("TouchpadStateMachine getParam failed");
    return -1;
  }
  s->print("param\n  gain:");
  s->print((int)i2cdata_param.gain);
  s->print(", prescale:");
  s->print((int)i2cdata_param.prescale);
  s->print(", oversamples:");
  s->print((int)i2cdata_param.oversamples);
  s->print(", rsel:");
  s->print((int)i2cdata_param.rsel);
  s->print(", dzx*10:");
  s->print((int)i2cdata_param.deadzone_x);
  s->print(", dzy*10:");
  s->print((int)i2cdata_param.deadzone_y);
  s->print("\n");
  return 0;
}

int TouchpadStateMachine::showMsg(Stream *s) {
  if(!isAvailable()) return -1;
  multicoremsg_t msg;

  while(1) {
    uint32_t r;
    msg.cmd = I2CCtrl::GET_TPMSG;
    msg.addr = getAddr();
    msg.buf = (uint8_t *)&i2cdata_msg;
    msg.bufsize = sizeof(i2cdata_msg);
    multicoreMsgPush(msg);
    multicoreMsgPop(msg);
    if(msg.result != 0) {
      LOG_ERROR("TouchpadStateMachine showMsg failed:", msg.result);
      return -1;
    }
    if(i2cdata_msg.len == 0) {
      break;
    }
    uint8_t p = 0;
    while(i2cdata_msg.len) {
      s->write(i2cdata_msg.data[p++]);
      i2cdata_msg.len--;
    }
  }

  return 0;
}

int TouchpadStateMachine::getVersion(I2CVersionInfo &vinfo) {
  if(!isAvailable()) return -1;
  multicoremsg_t msg;

  msg.cmd = I2CCtrl::GET_VERSION;
  msg.addr = getAddr();
  msg.buf = (uint8_t *)&vinfo;
  msg.bufsize = sizeof(vinfo);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  if(msg.result != 0) {
    LOG_ERROR("TouchpadStateMachine getVersion failed:", msg.result);
    return -1;
  }

  return 0;
}


bool TouchpadStateMachine::checkTransition(bool tapadv) {
  if(!isAvailable()) return false;

  // don't care buttons
  anyvalue = TP_SET_TOUCH(tpdata.touch & I2CTPData_TOUCH) | TP_SET_X(tpdata.x) | TP_SET_Y(tpdata.y);
  state = state_prev = KS_VALUE;
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG(i2c_addr, ": touch=", (int)(tpdata.touch & I2CTPData_TOUCH), ", x=", (int)tpdata.x, ", y=", (int)tpdata.y);
  }
#endif
  return(tpdata.touch & I2CTPData_TOUCH);
}
/*
bool TouchpadStateMachine::checkTransition(bool tapadv) {
  F_Mouse::Movement mv;
  bool moving = false;
  unsigned long now = millis();
  unsigned long diff = now - last_transition;
  state = state_prev = KS_VALUE;

  if(diff < 10) {
    anyvalue = 0;
    return false;
  }

  last_transition = now;
  if(tpdata.touch) {
    moving = true;
    if(tpdata_prev.touch) { // moving
      x_delta = (x_delta *2 + (tpdata.x - tpdata_prev.x)) / 3;
      y_delta = (y_delta *2 + (tpdata.y - tpdata_prev.y)) / 3;
      mv.x = x_delta;
      mv.y = y_delta;
    } else { // first touch, reset movement
      mv.x = 0;
      mv.y = 0;
    }
    anyvalue = mv.value32;
  } else {
    moving = false;
    anyvalue = 0;
  }
  tpdata_prev = tpdata;

  return moving;
}
*/

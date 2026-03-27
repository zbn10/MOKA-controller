#include <Arduino.h>
#include <Wire.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "mcu.h"
#include "custom_keymap.h"

//appcontext_sys_t &ctxsys = appctxSys();

const uint8_t KeySensorMCU::keypos2mcu[DEFS_NMAGKEYS] = {
#if defined(SUPPORT_MCU_012345)
  0, 0, 0, 1, 1, 2,   3, 4, 4, 5, 5, 5,
  0, 0, 1, 1, 1, 2,   3, 4, 4, 4, 5, 5,
  0, 0, 1, 1, 2, 2,   3, 3, 4, 4, 5, 5,
           2, 2, 2,   3, 3, 3
#endif
#if defined(SUPPORT_MCU_012543)
  0, 0, 0, 1, 1, 2,   5, 4, 4, 3, 3, 3,
  0, 0, 1, 1, 1, 2,   5, 4, 4, 4, 3, 3,
  0, 0, 1, 1, 2, 2,   5, 5, 4, 4, 3, 3,
           2, 2, 2,   5, 5, 5
#endif
};

const uint8_t KeySensorMCU::keypos2key[DEFS_NMAGKEYS] = {
#if defined(SUPPORT_MCU_012345)
  0, 3, 6, 2, 5, 1,   5, 1, 4, 0, 3, 6,
  1, 4, 0, 3, 6, 2,   4, 0, 3, 6, 2, 5,
  2, 5, 1, 4, 0, 3,   3, 6, 2, 5, 1, 4,
           4, 5, 6,   0, 1, 2
#endif
#if defined(SUPPORT_MCU_012543)
  0, 3, 6, 2, 5, 1,   1, 5, 2, 6, 3, 0,
  1, 4, 0, 3, 6, 2,   2, 6, 3, 0, 4, 1,
  2, 5, 1, 4, 0, 3,   3, 0, 4, 1, 5, 2,
           4, 5, 6,   6, 5, 4
#endif
};

const uint8_t KeySensorMCU::mcukey2keypos[MCU_NMCUS][I2CCtrl::NKEYS] = {
#if defined(SUPPORT_MCU_012345)
  {0, 12, 24, 1, 13, 25, 2},
  {14, 26, 3, 15, 27, 4, 16},
  {28, 5, 17, 29, 36, 37, 38},
  {39, 40, 41, 30, 18, 6, 31},
  {19, 7, 32, 20, 8, 33, 21},
  {9, 34, 22, 10, 35, 23, 11}
#endif
#if defined(SUPPORT_MCU_012543)
  {0, 12, 24, 1, 13, 25, 2},
  {14, 26, 3, 15, 27, 4, 16},
  {28, 5, 17, 29, 36, 37, 38},
  {11, 23, 35, 10, 22, 34, 9},
  {21, 33, 8, 20, 32, 7, 19},
  {31, 6, 18, 30, 41, 40, 39}
#endif
};

appcontext_mcu_t &appctxMcu() {
  static appcontext_mcu_t ctx = { {
    KeySensorMCU(0, I2CCtrl::MCU0_ADDR),
    KeySensorMCU(1, I2CCtrl::MCU1_ADDR),
    KeySensorMCU(2, I2CCtrl::MCU2_ADDR),
    KeySensorMCU(3, I2CCtrl::MCU3_ADDR),
    KeySensorMCU(4, I2CCtrl::MCU4_ADDR),
    KeySensorMCU(5, I2CCtrl::MCU5_ADDR),
  } };
  return ctx;
}

void KeySensorMCU::reflectKeyStateOnLED(KeyswStateMachine **ksms) {
  for(uint8_t mcu = 0; mcu < MCU_NMCUS; mcu++) {
    KeySensorMCU &ksmcu = ctxmcu.mcu[mcu];
    if(ksmcu.getError()) continue;

    bool f = false;
    uint8_t state[2] = {};

    for(int k = 0; k < I2CCtrl::NKEYS; k++) {
      MagswStateMachine *msm = (MagswStateMachine *)ksms[mcukey2keypos[mcu][k]];
      state[k/4] <<= 2;
      //if(msm->getErrorOrForceOff()) {
      //  state[k/4] |= I2CCtrl::KEY_RELEASE;
      //} else
      if(msm->isPhysicalStateChanged()) {
        f = true;
        uint8_t s = msm->getState();
        state[k/4] |= (s == KS_RELEASE ? I2CCtrl::KEY_RELEASE : (s == KS_PARTIAL ? I2CCtrl::KEY_PARTIALPRESS : I2CCtrl::KEY_PRESS));
      }
    }
#if defined(DEBUG)
    if(f && ctxsys.kb_ctrl.isDebugMode()) { LOG_DEBUG("mcu", mcu, ", state[0]", DebugLogBase::BIN,state[0], ", state[1]",DebugLogBase::BIN,state[1]); }
#endif
    if(f) ksmcu.setLEDState(state[0], state[1]); // ignore error
  }
}


void KeySensorMCU::setError() {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_ERROR("MCU ", getMCUNumber(), " becomes error state");
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "MCU Error", "detected");
  }
#endif
  error = true;
}

uint8_t KeySensorMCU::isAlive() {
  multicoremsg_t msg;
  msg.cmd = I2CCtrl::IS_ALIVE;
  msg.addr = getAddr();
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  return(msg.result);
}

int KeySensorMCU::setLEDState(uint8_t state0, uint8_t state1) {
  if(getError()) return -2;

  multicoremsg_t msg;
  msg.cmd = I2CCtrl::SET_LED_STATE;
  msg.addr = getAddr();
  msg.led_state[0] = state0;
  msg.led_state[1] = state1;
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  if(msg.result != 0) {
    LOG_ERROR("MCU ", getMCUNumber(), " endTransmission:", msg.result, " state:", state0,",",state1);
    setError();
    return -3;
  }

  return 1;
}

/*
int KeySensorMCU::setLEDAlert(uint8_t alert) {
  if(getError()) return -2;

  fifomsg_t msg;
  msg.b[0] = I2CCtrl::SET_LED_ALERT;
  msg.b[1] = getAddr();
  msg.b[2] = alert;
  rp2040.fifo.push(msg.i);

  uint32_t error = rp2040.fifo.pop();
  //
  // alert status bit0:key0, bit6:key6
  //
  if(error != 0) {
    LOG_ERROR("MCU ", getMCUNumber(), " endTransmission: ", error);
    setError();
    return -3;
  }

  return 1;
}
*/

int KeySensorMCU::setLEDAttributesAll(I2CLEDAttributes &la) {
  multicoremsg_t msg;

  msg.cmd = I2CCtrl::SET_LED_ATTRIBUTES;
  msg.buf = (uint8_t *)&la;
  msg.bufsize = sizeof(la);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  if(msg.result != 0) {
    LOG_ERROR("MCU (broadcast), endTransmission: ", msg.result);
    return -3;
  }

  return 1;
}

int KeySensorMCU::calibrationMode(bool b) {
  multicoremsg_t msg;
  msg.cmd = (b ? I2CCtrl::CALIBRATION_MODE_ENTER : I2CCtrl::CALIBRATION_MODE_EXIT);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  if(msg.result != 0) {
    LOG_ERROR("calibration mode ", (b ? "enter" : "exit"), " error:", msg.result);
    return -3;
  }

  return 1;
}

int KeySensorMCU::calibrationPress(uint8_t key) {
  if(key >= I2CCtrl::NKEYS) return -1;
  if(getError()) return -2;

  multicoremsg_t msg;
  msg.cmd = I2CCtrl::CALIBRATION_PRESS;
  msg.addr = getAddr();
  msg.calibration_key = key;
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  if(msg.result != 0) {
    LOG_ERROR("MCU ", getMCUNumber(), ", failed, endTransmission:", msg.result);
    setError();
    return -3;
  }

  return 1;
}

int KeySensorMCU::resetEEPROM() {
  if(getError()) return -2;

  multicoremsg_t msg;
  msg.cmd = I2CCtrl::RESET_EEPROM;
  msg.addr = getAddr();
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  if(msg.result != 0) {
    LOG_ERROR("MCU ", getMCUNumber(), ", failed, endTransmission:", msg.result);
    setError();
    return -3;
  }

  return 1;
}

int KeySensorMCU::resetEEPROMAll() {
  multicoremsg_t msg;
  msg.cmd = I2CCtrl::RESET_EEPROM;
  msg.addr = 0; // broadcast
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  if(msg.result != 0) {
    LOG_ERROR("failed, endTransmission:", msg.result);
    return -3;
  }

  return 1;
}

int KeySensorMCU::calibrationReleaseAll() {
  multicoremsg_t msg;
  msg.cmd = I2CCtrl::CALIBRATION_RELEASE;
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  /*
  Wire.beginTransmission(0);
  Wire.write(I2CCtrl::RELEASE_CALIBRATION);
  int error = Wire.endTransmission();
  */
  if(msg.result != 0) {
    LOG_ERROR("endTransmission error:", msg.result);
    return -3;
  }

  return 1;
}

int KeySensorMCU::getRawval(uint8_t i2c, uint16_t (&val)[I2CCtrl::NKEYS]) {
  if(i2c != I2CCtrl::GET_RAWVAL && i2c != I2CCtrl::GET_RAWRELEASE && i2c != I2CCtrl::GET_RAWPRESS) return -1;
  if(getError()) return -2;

  uint8_t buf[I2CCtrl::NKEYS * sizeof(uint16_t)];
  multicoremsg_t msg;
  msg.cmd = i2c;
  msg.addr = getAddr();
  msg.buf = buf;
  msg.bufsize = sizeof(buf);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  if(msg.result != 0) {
    LOG_ERROR("MCU ", getMCUNumber(), ", core1 processing failed:", msg.result);
    setError();
    return -3;
  }

  int p = 0;
  for(int i = 0; i < I2CCtrl::NKEYS; i++) {
    val[i] = (buf[p++] << 8);
    val[i] += buf[p++];
  }

  return 1;
}

int KeySensorMCU::getSeveralData(I2CMCUData *sdata) {
  if(getError()) return -2;

  multicoremsg_t msg;
  msg.cmd = I2CCtrl::GET_SEVERAL_DATA;
  msg.addr = getAddr();
  msg.buf = (uint8_t *)sdata;
  msg.bufsize = sizeof(I2CMCUData);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  return 1;
}

int KeySensorMCU::showMCUMessage(Stream *s) {
  if(getError()) return -2;

  I2CMaxData data;
  multicoremsg_t msg;

  uint32_t r;
  while(1) {
    msg.cmd = I2CCtrl::GET_MSG;
    msg.addr = getAddr();
    msg.buf = (uint8_t *)&data;
    msg.bufsize = sizeof(data);
    multicoreMsgPush(msg);
    multicoreMsgPop(msg);

    if(msg.result != 0) {
      LOG_ERROR("MCU ", getMCUNumber(), ", showMCUMessage failed:", msg.result);
      setError();
      return -1;
    }
    if(!data.len) {
      s->write('\n');
      return 0;
    }

    uint8_t p = 0;
    while(data.len) {
      s->write(data.data[p++]);
      data.len--;
    }
  }
}

int KeySensorMCU::getMCUVersion(I2CVersionInfo &vinfo) {
  if(getError()) return -2;

  multicoremsg_t msg;

  msg.cmd = I2CCtrl::GET_VERSION;
  msg.addr = getAddr();
  msg.buf = (uint8_t *)&vinfo;
  msg.bufsize = sizeof(vinfo);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  if(msg.result != 0) {
    LOG_ERROR("MCU ", getMCUNumber(), ", getMCUVersion failed:", msg.result);
    setError();
    return -1;
  }

  return 0;
}

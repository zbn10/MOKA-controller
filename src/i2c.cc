#include <Arduino.h> 
#include <Wire.h>
#include "debuglog.h"
#include <DebugLog.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "controller.h"
#include "sysutils.h"
#include "i2c_control.h"
#include "i2c.h"
#include "sysutils.h"
#include "utils.h"
#include "mcu.h"
#include "custom_keymap.h"
#include "keysw_statemachine.h"

appcontext_i2c_t &appctxI2C() {
  static appcontext_i2c_t ctx = {CircularBuffer(), timeMeasurement()};
  return ctx;
}

#if defined(SUPPORT_TOUCHPAD)
TouchpadStateMachine *tpsm[2] = {
  &ctxksm.tpsm_left,
  &ctxksm.tpsm_right
};
uint8_t tpsm_index = 0;
#endif

void resetPowerLine() {
#if defined(SUPPORT_LOADSWITCH_CH217K)
  digitalWrite(DEFS_PIN_RESET_MCUS, HIGH);
  delay(100);
  digitalWrite(DEFS_PIN_RESET_MCUS, LOW);
#elif defined(SUPPORT_LOADSWITCH_TCK107AF)
  digitalWrite(DEFS_PIN_RESET_MCUS, LOW);
  delay(100);
  digitalWrite(DEFS_PIN_RESET_MCUS, HIGH);
#else
#error "Wrong LOADSWITCH defined"
#endif
  delay(100);
}

void setup1() {
  WireMCU.setSDA(DEFS_PIN_WIREMCU_SDA);
  WireMCU.setSCL(DEFS_PIN_WIREMCU_SCL);
  Wire2nd.setSDA(DEFS_PIN_WIRE2ND_SDA);
  Wire2nd.setSCL(DEFS_PIN_WIRE2ND_SCL);

  // reset MCUs
  pinMode(DEFS_PIN_RESET_MCUS, OUTPUT);
  delay(1);
  resetPowerLine();

  WireMCU.setClock(DEFS_I2CCLOCK);
  WireMCU.setTimeout(I2CCtrl::TIMEOUT);
  WireMCU.begin();
  Wire2nd.setClock(DEFS_I2C2NDCLOCK);
  Wire2nd.setTimeout(I2CCtrl::TIMEOUT);
  Wire2nd.begin();

  Wire2nd.beginTransmission(I2CCtrl::SSD1306_ADDR);
  ctxsys.kb_ctrl.enableSSD1306(Wire2nd.endTransmission() ? false : true);
  if(ctxsys.kb_ctrl.isSSD1306()) {
    ctxsys.oled.begin(I2CCtrl::SSD1306_ADDR);
    ctxsys.oled.clearDisplay();
    ctxsys.oled.display();
    delay(10);
  }

#if defined(SUPPORT_TOUCHPAD)
  Wire2nd.beginTransmission(I2CCtrl::TPL_ADDR);
  ctxksm.tpsm_left.setAvailable(Wire2nd.endTransmission() ? false : true);
  Wire2nd.beginTransmission(I2CCtrl::TPR_ADDR);
  ctxksm.tpsm_right.setAvailable(Wire2nd.endTransmission() ? false : true);
#endif

  rp2040.fifo.pop();
  rp2040.fifo.push(I2CCtrl::READY);
}

#if defined(DEBUG)
unsigned long loop1_prev_ms = 0;
#endif
void loop1() {
  multicoremsg_t msg;
  uint8_t rcvd = 0;
  bool i2c_period = false;
  unsigned long now_ms = millis();
  unsigned long diff_ms = now_ms - loop1_prev_ms;
  if(ctxsys.kb_ctrl.isSlowMode() && diff_ms >= ctxsys.kb_ctrl.getSlowModeDelay()) {
    i2c_period = true;
    loop1_prev_ms = now_ms;
  }

#if defined(DEBUG)
  ctxi2c.i2cloop.measurement(0);
#endif

  if(multicoreMsgPop(msg, true)) { // non blocking
    switch(msg.cmd) {
      case I2CCtrl::SET_LED_STATE:
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(I2CCtrl::SET_LED_STATE);
        WireMCU.write(msg.led_state[0]);
        WireMCU.write(msg.led_state[1]);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::GET_TPRAWVAL:
      case I2CCtrl::GET_TPMSG:
      case I2CCtrl::GET_TPPARAM:
        Wire2nd.beginTransmission(msg.addr);
        Wire2nd.write(msg.cmd);
        msg.result = Wire2nd.endTransmission();
        if(msg.result != 0) {
          ctxi2c.debug_loop1.print("[ERROR] loop1() GET_TP* endTransmission: ");
          ctxi2c.debug_loop1.print((int)msg.result);
          ctxi2c.debug_loop1.print('\n');
          break;
        }
        rcvd = Wire2nd.requestFrom(msg.addr, msg.bufsize);
        if(rcvd != msg.bufsize) {
          ctxi2c.debug_loop1.print("[ERROR] loop1() GET_TP* requestFrom: ");
          ctxi2c.debug_loop1.print((int)rcvd);
          ctxi2c.debug_loop1.print('\n');
          msg.result = 1;
          break;
        }
        if(!msg.buf) {
          ctxi2c.debug_loop1.print("[ERROR] loop1() GET_TP* : null buf\n");
          msg.result = 2;
        } else {
          uint8_t *ptr = msg.buf;
          for(int i = 0; i < msg.bufsize; i++) {
            *ptr++ = (uint8_t)Wire2nd.read();
          }
          msg.result = 0;
        }
        break;

      case I2CCtrl::SET_TPPARAM:
        Wire2nd.beginTransmission(msg.addr);
        Wire2nd.write(I2CCtrl::SET_TPPARAM);
        Wire2nd.write(msg.buf, msg.bufsize);
        msg.result = Wire2nd.endTransmission();
        break;

      case I2CCtrl::OLED: // display, scroll, invertdisplay, dim
        if(!ctxsys.kb_ctrl.isSSD1306()) {
          msg.result = 1;
          break;
        }
        msg.result = 0;
        switch(msg.oled_cmd) {
          case OLED::CMD_DISPLAY:
            ctxsys.oled.display();
            break;

          case OLED::CMD_SCROLLSTART:
          case OLED::CMD_SCROLLSTOP:
          case OLED::CMD_INVERTDISPLAY:
          case OLED::CMD_DIM:
          default:
            msg.result = 10;
        }
        break;

      /*
      case I2CCtrl::SET_LED_ALERT:
        WireMCU.beginTransmission(msg.b[1]);
        WireMCU.write(I2CCtrl::SET_LED_ALERT);
        WireMCU.write(msg.b[2]);
        err = WireMCU.endTransmission();
        break;
      */

      case I2CCtrl::SET_LED_ATTRIBUTES:
        WireMCU.beginTransmission(0);
        WireMCU.write(I2CCtrl::SET_LED_ATTRIBUTES);
        WireMCU.write(msg.buf, msg.bufsize);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::CALIBRATION_MODE_ENTER:
      case I2CCtrl::CALIBRATION_MODE_EXIT:
        WireMCU.beginTransmission(0);
        WireMCU.write(msg.cmd);
        msg.result = WireMCU.endTransmission();
        Wire2nd.beginTransmission(0);
        Wire2nd.write(msg.cmd);
        msg.result = msg.result * 10 + Wire2nd.endTransmission();
        break;

      case I2CCtrl::CALIBRATION_RELEASE:
        WireMCU.beginTransmission(0);
        WireMCU.write(msg.cmd);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::CALIBRATION_PRESS:
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(I2CCtrl::CALIBRATION_PRESS);
        WireMCU.write(msg.calibration_key);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::RESET_EEPROM:
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(I2CCtrl::RESET_EEPROM);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::GET_RAWVAL:
      case I2CCtrl::GET_RAWRELEASE:
      case I2CCtrl::GET_RAWPRESS:
      case I2CCtrl::GET_SEVERAL_DATA:
        if(!msg.buf) break;
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(msg.cmd);
        msg.result = WireMCU.endTransmission();
        if(msg.result != 0) break;
        rcvd = WireMCU.requestFrom(msg.addr, msg.bufsize);
        if(rcvd != msg.bufsize) { msg.result = 1; break; } else { msg.result = 0; }
        for(int i = 0; i < msg.bufsize; i++) {
          (msg.buf)[i] = WireMCU.read();
        }
        break;

      case I2CCtrl::GET_VERSION:
        if(msg.buf) {
          TwoWire &wire = isTP(msg.addr) ? Wire2nd : WireMCU;
          wire.beginTransmission(msg.addr);
          wire.write(msg.cmd);
          msg.result = wire.endTransmission();
          if(msg.result != 0) break;
          rcvd = wire.requestFrom(msg.addr, msg.bufsize);
          if(rcvd != msg.bufsize) { msg.result = 1; break; } else { msg.result = 0; }
          for(int i = 0; i < msg.bufsize; i++) {
            (msg.buf)[i] = wire.read();
          }
        }
        break;

#if defined(DEBUG)
      case I2CCtrl::GET_LOOPMEASURE:
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(I2CCtrl::GET_LOOPMEASURE);
        msg.result = WireMCU.endTransmission();
        if(msg.result != 0) break;
        rcvd = WireMCU.requestFrom(msg.addr, msg.bufsize);
        if(rcvd != msg.bufsize) { msg.result = 2; break; }
        else {
          for(uint8_t i = 0; i < msg.bufsize; i++) {
            msg.buf[i] = WireMCU.read();
          }
          /*
          err = (WireMCU.read() << 8);
          err += WireMCU.read();
          */
        }
        while(WireMCU.available()) { WireMCU.read(); }
        break;

      case I2CCtrl::SET_KEYTEST_VAL:
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(I2CCtrl::SET_KEYTEST_VAL);
        WireMCU.write(msg.buf, msg.bufsize);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::SET_KEYTEST_STOP:
        WireMCU.beginTransmission(0);
        WireMCU.write(I2CCtrl::SET_KEYTEST_STOP);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::GET_MSG:
        WireMCU.beginTransmission(msg.addr);
        WireMCU.write(I2CCtrl::GET_MSG);
        msg.result = WireMCU.endTransmission();
        if(msg.result != 0) break;
        rcvd = WireMCU.requestFrom(msg.addr, msg.bufsize);
        for(int i = 0; i < msg.bufsize; i++) {
          msg.buf[i] = WireMCU.read();
        }
        while(WireMCU.available()) { WireMCU.read(); }
        break;

      case I2CCtrl::SET_DEBUG:
        WireMCU.beginTransmission(0);
        WireMCU.write(msg.cmd);
        WireMCU.write(msg.debug_mode);
        msg.result = WireMCU.endTransmission();
        Wire2nd.beginTransmission(0);
        Wire2nd.write(msg.cmd);
        Wire2nd.write(msg.debug_mode);
        msg.result = msg.result * 10 + Wire2nd.endTransmission();
        break;

      case I2CCtrl::I2C_DMSG:
        ctxi2c.debug_loop1.show();
        msg.result = 0;
        break;
#endif

      case I2CCtrl::SCAN:
        if(msg.bufsize < 2 * 127) {
          msg.result = 1;
          break;
        }
        for (int address = 1; address < 127; address++ ) {
          WireMCU.beginTransmission(address);
          msg.buf[address] = WireMCU.endTransmission();
        }
        for (int address = 1; address < 127; address++ ) {
          Wire2nd.beginTransmission(address);
          msg.buf[address + 127] = Wire2nd.endTransmission();
        }
        msg.result = 0;
        break;

      case I2CCtrl::IS_ALIVE:
        WireMCU.beginTransmission(msg.addr);
        msg.result = WireMCU.endTransmission();
        break;

      case I2CCtrl::RESET:
        Wire2nd.beginTransmission(msg.addr);
        Wire2nd.write(I2CCtrl::RESET);
        msg.result = Wire2nd.endTransmission();
        break;

      case I2CCtrl::MCURESET:
        resetPowerLine();
        msg.result = 0;
        break;

      default:
        msg.result = 10;
    }
    rp2040.fifo.push(msg.ui32[0]);
    rp2040.fifo.push(msg.ui32[1]);
  }


  else {
    // MagswStateMachine
    bool key_error_presence = false;
    for(int mcu = 0; mcu < MCU_NMCUS; mcu++) {
      if(ctxmcu.mcu[mcu].getError()) {
        for(uint8_t k = 0; k < I2CCtrl::NKEYS; k++) {
          uint8_t keypos = KeySensorMCU::mcukey2keypos[mcu][k];
          CustomKeymap::ksm[keypos]->setKeyval(0);
        }
        continue;
      }

      // stage0
      rcvd = WireMCU.requestFrom(ctxmcu.mcu[mcu].getAddr(), sizeof(I2CKeyState));
      if(!rcvd) {
        ctxi2c.debug_loop1.print("[ERROR] loop1() Stage0 rcvd: ");
        ctxi2c.debug_loop1.print(rcvd);
        ctxi2c.debug_loop1.print(", MCU ");
        ctxi2c.debug_loop1.print(mcu);
        ctxi2c.debug_loop1.print('\n');
        ctxmcu.mcu[mcu].setError();
        continue;
      }
      I2CKeyState keystate;
      keystate.err = WireMCU.read();
      keystate.pressed = WireMCU.read();
      uint8_t cnt = 0, b = 1;
      for(uint8_t k = 0; k < I2CCtrl::NKEYS; k++) {
        if(keystate.pressed & b) cnt++;
        b <<= 1;
      }
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode() && i2c_period) {
        ctxi2c.debug_loop1.print("[DEBUG] loop1() Stage0 MCU ");
        ctxi2c.debug_loop1.print(mcu);
        ctxi2c.debug_loop1.print(": err= 0b");
        ctxi2c.debug_loop1.print(keystate.err, BIN);
        ctxi2c.debug_loop1.print(", pressed= 0b");
        ctxi2c.debug_loop1.print(keystate.pressed, BIN);
        ctxi2c.debug_loop1.print(", count=");
        ctxi2c.debug_loop1.print(cnt);
        ctxi2c.debug_loop1.print('\n');
      }
#endif

      // stage1
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode() && i2c_period) {
        ctxi2c.debug_loop1.print("[DEBUG] loop1() Stage1 MCU ");
      }
#endif
      if(cnt) {
        rcvd = WireMCU.requestFrom(ctxmcu.mcu[mcu].getAddr(), cnt);
        if(!rcvd) {
          ctxi2c.debug_loop1.print("[ERROR] loop1() Stage1 rcvd: ");
          ctxi2c.debug_loop1.print(rcvd);
          ctxi2c.debug_loop1.print(", MCU ");
          ctxi2c.debug_loop1.print(mcu);
          ctxi2c.debug_loop1.print('\n');
          ctxmcu.mcu[mcu].setError();
          continue;
        }
      }

      if(keystate.err) key_error_presence = true;
      for(uint8_t k = 0; k < I2CCtrl::NKEYS; k++) {
        uint8_t keypos = KeySensorMCU::mcukey2keypos[mcu][k];
        CustomKeymap::ksm[keypos]->setKeyval(keystate.pressed & 1 ? WireMCU.read() : 0);
        keystate.pressed >>= 1;
        CustomKeymap::ksm[keypos]->setError(keystate.err & 1);
        keystate.err >>= 1;
#if defined(DEBUG)
        if(ctxsys.kb_ctrl.isDebugMode() && i2c_period) {
          ctxi2c.debug_loop1.print(k);
          ctxi2c.debug_loop1.print('/');
          ctxi2c.debug_loop1.print(CustomKeymap::ksm[keypos]->getKeyval());
          ctxi2c.debug_loop1.print('/');
          ctxi2c.debug_loop1.print(CustomKeymap::ksm[keypos]->getError());
          ctxi2c.debug_loop1.print(", ");
        }
#endif
      }
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode() && i2c_period) {
        ctxi2c.debug_loop1.print("\n");
      }
#endif
    }
    MagswStateMachine::setKeyErrorPresence(key_error_presence);

#if defined(SUPPORT_TOUCHPAD)
#if defined(DEBUG)
    ctxi2c.i2cloop.measurement(1);
#endif

// TouchpadStateMachine
    if(tpsm[tpsm_index]->isAvailable()) {
      rcvd = Wire2nd.requestFrom(tpsm[tpsm_index]->getAddr(), sizeof(I2CTPData));
      if(!rcvd) {
        if(ctxsys.kb_ctrl.isPeriod()) {
          ctxi2c.debug_loop1.print("[ERROR] loop1() tpsm rcvd: ");
          ctxi2c.debug_loop1.print(rcvd);
          ctxi2c.debug_loop1.print(", TP ");
          ctxi2c.debug_loop1.print(tpsm_index);
          ctxi2c.debug_loop1.print('\n');
        }
      }
  
      else {
        uint8_t *ptr = (uint8_t *)&(tpsm[tpsm_index]->tpdata);
        for(int i = 0; i < sizeof(I2CTPData); i++) {
          ptr[i] = Wire2nd.read();
        }
      }
    }

    tpsm_index = (tpsm_index + 1) % (sizeof(tpsm)/sizeof(tpsm[0]));
#endif

  }
}

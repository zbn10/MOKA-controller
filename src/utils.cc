#include <Arduino.h>
#include <Stream.h>
#include <Wire.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "hid.h"
#include "sysutils.h"
#include "mcu.h"
#include "keysw_statemachine.h"
#include "custom_keymap.h"
#include "config.h"
#include "utils.h"
#include "repl.h"
#include "i2c.h"
#include <Adafruit_NeoPixel.h>


void Utils::i2cScanner(Stream *s) {
  int address, error;
  if(s == nullptr) return;

  multicoremsg_t msg;
  msg.cmd = I2CCtrl::SCAN;
  msg.buf = (uint8_t *)scan_result;
  msg.bufsize = sizeof(scan_result);
  multicoreMsgPush(msg);
  multicoreMsgPop(msg);

  s->print("# on WireMCU\n");
  for (address = 1; address < 127; address++ ) {
    switch(scan_result[0][address]) {
      case 0:
        s->print("# I2C device found at address 0x");
        s->print(address, HEX);
        s->print("  !\n");
        break;
      case 4:
        s->print("# Unknown error at address 0x");
        s->print(address, HEX);
        s->print('\n');
      case 5:
        s->print("# No device at address 0x");
        s->print(address, HEX);
        s->print('\n');
    }
  }
  s->print("# on Wire2nd\n");
  for (address = 1; address < 127; address++ ) {
    switch(scan_result[1][address]) {
      case 0:
        s->print("# I2C device found at address 0x");
        s->print(address, HEX);
        s->print("  !\n");
        break;
      case 4:
        s->print("# Unknown error at address 0x");
        s->print(address, HEX);
        s->print('\n');
      case 5:
        s->print("# No device at address 0x");
        s->print(address, HEX);
        s->print('\n');
    }
  }
}

const char *Utils::calibrationCmd = "cl";
const char *Utils::calibrationSyntax = "cl 1(enter) | 0(exit) | r(elease) | p(ress) 0-5(MCU) 0-6(KEY)";
const char *Utils::calibrationDescr = "Calibrate magnetic key sensors";
int Utils::calibration(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;

  uint8_t mcu, key;
  int r;
  switch(*av[0]) {
    case '0': // exit calibration mode
      if(!ctxsys.kb_ctrl.isCalibrationMode()) {
        s->print("# already in normal mode\n");
        return 0;
      }
      s->print("# exit calibration mode\n");
      F_SelectLayer::exitCalibration();
      break;

    case '1': // enter calibration mode
      if(ctxsys.kb_ctrl.isCalibrationMode()) {
        s->print("# already in calibration mode\n");
        return 0;
      }
      s->print("# enter calibration mode\n");
      F_SelectLayer::enterCalibration();
      break;

    case 'r': // release calibration
      s->print("# release calibration of all MCUs\n");
      r = KeySensorMCU::calibrationReleaseAll();
      break;

    case 'p': // press calibration
      if(ac < 3) return -1;
      mcu = strtol(av[1], nullptr, 0);
      key = strtol(av[2], nullptr, 0);
      if(mcu >= MCU_NMCUS || key >= I2CCtrl::NKEYS) {
        s->print("# invalid MCU or KEY number\n");
        return -1;
      }
      s->print("# Press Calibration, MCU ");
      s->print(mcu);
      s->print(", key ");
      s->print(key);
      s->print('\n');
      r = ctxmcu.mcu[mcu].calibrationPress(key);
      break;

    default:
      s->print("# invalid argument\n");
      return -1;
  }

  if(r < 0) {
    s->print("# failed, r = ");
    s->print(r);
    s->print('\n');
    return -1;
  }
  return 1;
}

const char *Utils::keyvalCmd = "keyval";
const char *Utils::keyvalSyntax = "keyval 0-5(MCU)\nkeyval t(actsw) | p(hotoic) | i(nfrared)\nkeyval l(eft TP) | r(ight TP)";
const char *Utils::keyvalDescr = "Show keyval, rawval, rawrelease, rawpress, alert values on MCU and Trackpad";
int Utils::keyval(Stream *s, uint8_t ac, const char **av) {
  uint8_t i2c_ctrl;

  if(s == nullptr) return -1;
  if(ac < 1) return -1;

  switch(*av[0]) { // other than mag switches
    case 't':
      s->print("# TactSW keyval : ");
      s->print((int)ctxksm.tact_sw.getKeyval());
      s->print('\n');
      return 1;

    case 'p':
      s->print("# PhotoIC val : ");
#if defined(SUPPORT_PHOTOIC_INFRARED)
      s->print(ctxksm.photoic_sw.getValue());
#else
      s->print("None");
#endif
      s->print('\n');
      return 1;

    case 'i':
      s->print("# Infrared Sensor state : ");
#if defined(SUPPORT_PHOTOIC_INFRARED)
      s->print(ctxksm.infrared_sw.getValue());
      s->print(", ");
      s->print(ctxksm.infrared_sw.getAnyValue());
#else
      s->print("None");
#endif
      s->print('\n');
      return 1;

    case 'l':
    case 'r':
      s->print("# Touchpad val : ");
#if defined(SUPPORT_TOUCHPAD)
      if(*av[0] == 'l') ctxksm.tpsm_left.showVal(s);
      else ctxksm.tpsm_right.showVal(s);
#else
      s->print("None");
#endif
      s->print('\n');
      return 1;
  }

  uint8_t mcu = strtol(av[0], nullptr, 0);
  if(mcu >= MCU_NMCUS) return -1;

  if(ctxmcu.mcu[mcu].getError()) {
    s->print("# MCU ");
    s->print(mcu);
    s->print(" is in error state \n");
    return -1;
  }
  s->print("# MCU ");
  s->print(mcu);
  s->print('\n');

  s->print("# val ");
  for(int k = 0; k < I2CCtrl::NKEYS; k++) {
    s->print((int)(CustomKeymap::ksm[KeySensorMCU::mcukey2keypos[mcu][k]]->getKeyval()));
    s->print(',');
  }
  //s->print(" MCU keyerr = 0b");
  //s->print(ctxmcu.mcu[mcu].getMCUKeyerr(), BIN);
  s->print('\n');

  uint16_t rawval[I2CCtrl::NKEYS];
  int r = ctxmcu.mcu[mcu].getRawval(I2CCtrl::GET_RAWVAL, rawval);
  if(r < 0) {
    s->print("# GET_RAWVAL: MCU ");
    s->print(mcu);
    s->print(" is in error state ");
    s->print(r);
    s->print('\n');
    return -1;
  }
  s->print("# rawval ");
  for(int i = 0; i < I2CCtrl::NKEYS; i++) {
    s->print(rawval[i]);
    s->print(',');
  }
  s->print('\n');

  r = ctxmcu.mcu[mcu].getRawval(I2CCtrl::GET_RAWRELEASE, rawval);
  if(r < 0) {
    s->print("# GET_RAWRELEASE: MCU ");
    s->print(mcu);
    s->print(" is in error state ");
    s->print(r);
    s->print('\n');
    return -1;
  }
  s->print("# rawrelease ");
  for(int i = 0; i < I2CCtrl::NKEYS; i++) {
    s->print(rawval[i]);
    s->print(',');
  }
  s->print('\n');

  r = ctxmcu.mcu[mcu].getRawval(I2CCtrl::GET_RAWPRESS, rawval);
  if(r < 0) {
    s->print("# GET_RAWPRESS: MCU ");
    s->print(mcu);
    s->print(" is in error state ");
    s->print(r);
    s->print('\n');
    return -1;
  }
  s->print("# rawpress ");
  for(int i = 0; i < I2CCtrl::NKEYS; i++) {
    s->print(rawval[i]);
    s->print(',');
  }
  s->print('\n');

  I2CMCUData sdata;
  r = ctxmcu.mcu[mcu].getSeveralData(&sdata);
  if(r < 0) {
    s->print("# GET_SEVERAL_DATA: MCU ");
    s->print(mcu);
    s->print(" is in error state ");
    s->print(r);
    s->print('\n');
    return -1;
  }
  s->print("# release ");
  s->print(sdata.leddata.ledattr.led_ptn_release);
  s->print(", partialpress ");
  s->print(sdata.leddata.ledattr.led_ptn_partialpress);
  s->print(", press ");
  s->print(sdata.leddata.ledattr.led_ptn_press);
  s->print(", alert ");
  s->print(sdata.leddata.ledattr.led_ptn_alert);
  s->print(", backlight ");
  s->print(sdata.leddata.ledattr.led_backlight);
  s->print(", brightness ");
  s->print(sdata.leddata.ledattr.led_brightness);
  s->print(", backlight_enable ");
  s->print(sdata.leddata.ledattr.backlight_enable);
  s->print("\n# state ");
  for(int i = 0; i < I2CCtrl::NKEYS; i++) {
    s->print(sdata.leddata.state[i]);
    s->print(',');
  }
  s->print(", keytest b");
  s->print(sdata.keytest, BIN);
  s->print('\n');

  return 1;
}

void Utils::getMemUsage(Stream *s) {
  s->print("# Mem total:");
  s->print(rp2040.getTotalHeap());
  s->print(", used:");
  s->print(rp2040.getUsedHeap());
  s->print(", free:");
  s->print(rp2040.getFreeHeap());
  s->print('\n');
}

const char *Utils::ledCmd = "led";
const char *Utils::ledSyntax = "led k(eycolor) r(elease)|P(artial)|p(ress)|a(lert) <0-8>\n" \
  "led b(rightness) <num>\n" \
  "led B(acklight) [<num>] : on/off or backlight brightness\n" \
  "led c(olor) c(al)|s(ys)|m(cu)|k(ey)|N(umlock)|C(apslock)|S(crolllock) <r> <g> <b>\n" \
  "led d(ebug) c(al)|m(cu)|k(ey)|N(umlock)|C(apslock)|S(crolllock)";
const char *Utils::ledDescr = "Set RGB LED parameters";
int Utils::led(Stream *s, uint8_t ac, const char **av) {
  uint8_t r, g, b, type;

  if(ac < 1) return -1;
  switch(*av[0]) {
    case 'k':
      if(ac < 3) return -1;
      switch(*av[1]) {
        case 'r':
          ctxsys.f_systemled.setKeyLEDColor(KEYLED_RELEASE, strtol(av[2], nullptr, 0));
          break;
        case 'P':
          ctxsys.f_systemled.setKeyLEDColor(KEYLED_PARTIALPRESS, strtol(av[2], nullptr, 0));
          break;
        case 'p':
          ctxsys.f_systemled.setKeyLEDColor(KEYLED_PRESS, strtol(av[2], nullptr, 0));
          break;
        case 'a':
          ctxsys.f_systemled.setKeyLEDColor(KEYLED_ALERT, strtol(av[2], nullptr, 0));
          break;
        default:
          return -1;
      }
      break;

    case 'b':
      if(ac >= 2) {
        ctxsys.f_systemled.applyBrightness(strtol(av[1], nullptr, 0));
      } else {
        return -1;
      }
      break;

    case 'B':
      if(ac >= 2) {
        ctxsys.f_systemled.applyBacklight(strtol(av[1], nullptr, 0));
      } else {
        bool b = !ctxsys.f_systemled.isBacklightEnabled();
        ctxsys.f_systemled.enableBacklight(b);
        s->print("# Backlight ");
        s->print(b ? "enabled\n" : "disabled\n");
      }
      break;

    case 'c':
      if(ac < 5) return -1;
      switch(*av[1]) {
        case 'c':
          type = LEDTYPE_CALIBRATION;
          break;
        case 's':
          type = LEDTYPE_SYSTEM;
          break;
        case 'm':
          type = LEDTYPE_MCU;
          break;
        case 'k':
          type = LEDTYPE_KEY;
          break;
        case 'N':
          type = LEDTYPE_NUMLOCK;
          break;
        case 'C':
          type = LEDTYPE_CAPSLOCK;
          break;
        case 'S':
          type = LEDTYPE_SCROLLLOCK;
          break;
        default:
          s->print("# invalid option");
          return -1;
      }
      r = strtol(av[2], nullptr, 0);
      g = strtol(av[3], nullptr, 0);
      b = strtol(av[4], nullptr, 0);
      ctxsys.f_systemled.setLEDColor(type, r, g, b);
      break;

    case 'd':
      if(ac < 2) return -1;
      switch(*av[1]) {
        case 'c':
          ctxsys.kb_ctrl.enablePseudoSysFault(!ctxsys.kb_ctrl.isPseudoSysFault());
          break;
        case 'm':
          ctxsys.kb_ctrl.enablePseudoMCUFault(!ctxsys.kb_ctrl.isPseudoMCUFault());
          break;
        case 'k':
          ctxsys.kb_ctrl.enablePseudoKeyFault(!ctxsys.kb_ctrl.isPseudoKeyFault());
          break;
        case 'N':
          ctxsys.kb_ctrl.enablePseudoNumlock(!ctxsys.kb_ctrl.isPseudoNumlock());
          //ctxsys.f_systemled.setTypeState(F_SystemLED::TYPE_NUMLOCK, !ctxsys.f_systemled.getTypeState(F_SystemLED::TYPE_NUMLOCK));
          break;
        case 'C':
          ctxsys.kb_ctrl.enablePseudoCapslock(!ctxsys.kb_ctrl.isPseudoCapslock());
          //ctxsys.f_systemled.setTypeState(F_SystemLED::TYPE_CAPSLOCK, !ctxsys.f_systemled.getTypeState(F_SystemLED::TYPE_CAPSLOCK));
          break;
        case 'S':
          ctxsys.kb_ctrl.enablePseudoScrolllock(!ctxsys.kb_ctrl.isPseudoScrolllock());
          //ctxsys.f_systemled.setTypeState(F_SystemLED::TYPE_SCROLLLOCK, !ctxsys.f_systemled.getTypeState(F_SystemLED::TYPE_SCROLLLOCK));
          break;
        default:
          s->print("# invalid option\n");
          return -1;
      }
      break;

    default:
      s->print("# invalid option\n");
      return -1;
  }

  return 1;
}

const char *Utils::sysstatCmd = "sysstat";
const char *Utils::sysstatSyntax = "sysstat";
const char *Utils::sysstatDescr = "System Stat : mem usage, i2c devices";
int Utils::sysstat(Stream *s, uint8_t ac, const char **av) {
  getMemUsage(s);
  i2cScanner(s);
  //s->print("# mcu,err,keyflt: ");
  s->print("# MCU err: ");
  for(int i = 0; i < MCU_NMCUS; i++) {
    s->print(i);
    s->print(" ");
    s->print(ctxmcu.mcu[i].getError());
    //s->print(" ");
    //s->print(ctxmcu.mcu[i].getMCUKeyerr());
    s->print(" , ");
  }
  s->print('\n');

  s->print("# Key err: ");
  s->print(MagswStateMachine::getKeyErrorPresence());
  s->print(" (");
  for(int k = 0; k < DEFS_NMAGKEYS; k++) {
    if(CustomKeymap::ksm[k]->getError()) {
      s->print(k);
      s->print(" ");
    }
  }
  s->print(")");
  s->print('\n');

  s->print("# Protocol: Keyboard ");
  s->print(HidManager::getKeyboardProtocolFeature() ? "Feature" : "Boot");
  s->print('\n');

  s->print("# Operation mode: Factory ");
  s->print(ctxsys.kb_ctrl.isFactoryMode() ? "on" : "off");
  s->print(", Emergency ");
  s->print(ctxsys.kb_ctrl.isEmergencyMode() ? "on" : "off");
  s->print(", Calibration ");
  s->print(ctxsys.kb_ctrl.isCalibrationMode() ? "on" : "off");
  s->print(", Keytest ");
  s->print(ctxsys.kb_ctrl.isKeytestMode() ? "on" : "off");
  s->print('\n');

  s->print("# Devices: SSD1306 ");
  s->print(ctxsys.kb_ctrl.isSSD1306() ? "avail" : "unavail");
#if defined(SUPPORT_TINYMOUSE)
  s->print(", tmouse ");
  s->print(tmouse.isAvailable() ? "avail" : "unavail");
#endif
#if defined(SUPPORT_TOUCHPAD)
  s->print(", Touchpad L ");
  s->print(ctxksm.tpsm_left.isAvailable() ? "avail" : "unavail");
  s->print(", R ");
  s->print(ctxksm.tpsm_right.isAvailable() ? "avail" : "unavail");
  s->print(", mode ");
  s->print(F_Touchpad::getMouseMode() ? "mouse" : "touchpad");
  s->print('\n');
#endif

#if defined(DEBUG)
  s->print("# Debug: ");
  s->print(ctxsys.kb_ctrl.isDebugMode() ? "on" : "off");
  s->print(", slow ");
  s->print(ctxsys.kb_ctrl.getSlowModeDelay());
  s->print(", mcuerr ");
  s->print(ctxsys.kb_ctrl.isPseudoMCUFault() ? "on" : "off");
  s->print(", keyerr ");
  s->print(ctxsys.kb_ctrl.isPseudoKeyFault() ? "on" : "off");
  s->print('\n');

  s->print("# HID Report: keyboard ");
  s->print(ctxsys.kb_ctrl.isKeyboardReport() ? "on" : "off");
#if defined(SUPPORT_TINYMOUSE)
  s->print(", mouse ");
  s->print(ctxsys.kb_ctrl.isMouseReport() ? "on" : "off");
#endif
#if defined(SUPPORT_TOUCHPAD)
  s->print(", touchpad ");
  s->print(ctxsys.kb_ctrl.isTouchpadReport() ? "on" : "off");
#endif
  s->print('\n');

  s->print("# LED Debug: SYS ");
  s->print(ctxsys.kb_ctrl.isPseudoSysFault() ? "on" : "off");
  s->print(", MCU ");
  s->print(ctxsys.kb_ctrl.isPseudoMCUFault() ? "on" : "off");
  s->print(", Key ");
  s->print(ctxsys.kb_ctrl.isPseudoKeyFault() ? "on" : "off");
  s->print(", Num ");
  s->print(ctxsys.kb_ctrl.isPseudoNumlock() ? "on" : "off");
  s->print(", Caps ");
  s->print(ctxsys.kb_ctrl.isPseudoCapslock() ? "on" : "off");
  s->print(", Scroll ");
  s->print(ctxsys.kb_ctrl.isPseudoScrolllock() ? "on" : "off");
  s->print('\n');
#endif
  return 1;
}

const char *Utils::keyattrCmd = "keyattr";
const char *Utils::keyattrSyntax = "keyattr [k(ey) 0-41] <pp> <p> <pp2r> <p2r> <durhold>\n" \
  "keyattr [k(ey) 0-41] P|p|R|r|d <value>\n" \
  "  P(artial), p(ress), R(releasePartial), r(elease), d(ulation)\n" \
  "keyattr c(ontroll) 0-41(KEY) 0(normal)|1(forceoff)|2(forceon)";
const char *Utils::keyattrDescr = "Set key attributes on KEY";
int Utils::keyattr(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;

  uint8_t key, pp, p, pp2r, p2r;
  uint16_t dur_hold;

  if(*av[0] == 'c') {
    if(ac < 3) return -1;
    key = strtol(av[1], nullptr, 0);
    if(key >= DEFS_NMAGKEYS) {
      LOG_ERROR("Invalid key number");
      return -1;
    }
    uint8_t ctrl = strtol(av[2], nullptr, 0);
    if(ctrl >= KC_MAX) ctrl = 0;
    ((MagswStateMachine *)CustomKeymap::ksm[key])->setCtrl((KeyswCtrl)ctrl);
  }

  else if(*av[0] == 'k') {
    if(ac < 4) return -1;
    key = strtol(av[1], nullptr, 0);
    if(key >= DEFS_NMAGKEYS) {
      LOG_ERROR("Invalid key number");
      return -1;
    }

    pp = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPartialPress();
    p = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPress();
    pp2r = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPartial2Release();
    p2r = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPress2Release();
    dur_hold = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrDurationHold();
    switch(*av[2]) {
      case 'P':
        pp = strtol(av[3], nullptr, 0);
        break;
      case 'p':
        p = strtol(av[3], nullptr, 0);
        break;
      case 'R':
        pp2r = strtol(av[3], nullptr, 0);
        break;
      case 'r':
        p2r = strtol(av[3], nullptr, 0);
        break;
      case 'd':
        dur_hold = strtol(av[3], nullptr, 0);
        break;
      default: // full attributes
        if(ac < 7) return -1;
        pp = strtol(av[2], nullptr, 0);
        p = strtol(av[3], nullptr, 0);
        pp2r = strtol(av[4], nullptr, 0);
        p2r = strtol(av[5], nullptr, 0);
        dur_hold = strtol(av[6], nullptr, 0);
    }

    if(!((MagswStateMachine *)CustomKeymap::ksm[key])->setKeyattr(pp, p, pp2r, p2r, dur_hold)) {
      LOG_ERROR("Invalid key attributes");
      return -1;
    }
  } else {
    if(ac < 2) return -1;
    pp = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPartialPress();
    p = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPress();
    pp2r = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPartial2Release();
    p2r = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPress2Release();
    dur_hold = ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrDurationHold();
    bool r = true;
    uint8_t param = 0;
    switch(*av[0]) {
      case 'P':
        pp = strtol(av[1], nullptr, 0);
        param = 1<<0;
        break;
      case 'p':
        p = strtol(av[1], nullptr, 0);
        param = 1<<1;
        break;
      case 'R':
        pp2r = strtol(av[1], nullptr, 0);
        param = 1<<2;
        break;
      case 'r':
        p2r = strtol(av[1], nullptr, 0);
        param = 1<<3;
        break;
      case 'd':
        dur_hold = strtol(av[1], nullptr, 0);
        param = 1<<4;
        break;
      default: // full attributes
        if(ac < 5) return -1;
        pp = strtol(av[0], nullptr, 0);
        p = strtol(av[1], nullptr, 0);
        pp2r = strtol(av[2], nullptr, 0);
        p2r = strtol(av[3], nullptr, 0);
        dur_hold = strtol(av[4], nullptr, 0);
        param = 1<<0|1<<1|1<<2|1<<3|1<<4;
    }
    for(key = 0; key < DEFS_NMAGKEYS; key++) {
      if(!((MagswStateMachine *)CustomKeymap::ksm[key])->setKeyattr(
        (param & 1<<0 ? pp : ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPartialPress()),
        (param & 1<<1 ? p : ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPress()),
        (param & 1<<2 ? pp2r : ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPartial2Release()),
        (param & 1<<3 ? p2r : ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrPress2Release()),
        (param & 1<<4 ? dur_hold : ((MagswStateMachine *)CustomKeymap::ksm[key])->getKeyattrDurationHold())
      )) r = false;
    }
    if(!r) {
      LOG_ERROR("Invalid key attributes");
    }
  }

  return(1);
}

const char *Utils::keycodeCmd = "keycode";
const char *Utils::keycodeSyntax = "keycode\nkeycode l(ayout) 0(US)|1(JP)\n" \
  "keycode 0-46?(KEY) p|P|t|h|l|L|v(state) 0-4(layer) <func>\n" \
  "  p(ress), P(artial), t(ap), h(old), l(hold2), L(hold3), v(alue), Layer4 = Calibration\n" \
  "keycode t(apAdv)";
const char *Utils::keycodeDescr = "Set key function on KEY/layer";
int Utils::keycode(Stream *s, uint8_t ac, const char **av) {
  if(ac == 0) {
    for(uint16_t i = 0; i < 256; i++) {
      if(!ctxkeymap.keymap.getFunctionDisp(i)) continue;

      s->print(i);
      if(i < 100) s->print(' ');
      if(i < 10) s->print(' ');
      s->print(",  0x");
      if(i < 16) s->print('0');
      s->print(i, HEX);
      s->print(",  ");
      s->print(ctxkeymap.keymap.getFunctionDisp(i));
      s->print(",  ");
      s->print(ctxkeymap.keymap.getFunctionDescr(i));
      s->print('\n');
    }
    return(1);
  }

  if(*av[0] == 'l') {
    KeymapOSLayoutType layout = OSLAYOUT_US;
    if(ac < 2) return(-1);

    layout = (KeymapOSLayoutType)strtol(av[1], nullptr, 0);
    if(!ctxkeymap.keymap.validateOSLayout(layout)) layout = OSLAYOUT_US;
    ctxkeymap.keymap.setOSLayout(layout);

    s->print("# set OS Layout : ");
    s->print(ctxkeymap.keymap.getOSLayoutStr());
    s->print('\n');
    return(1);
  }

  if(*av[0] == 't') {
    bool b = !ctxkeymap.keymap.isTapAdvanced();
    ctxkeymap.keymap.enableTapAdvanced(b);
    s->print("# Tap Advanced ");
    s->print(b ? "enabled\n" : "disabled\n");
  }

  if(ac < 4) return(-1);
  uint8_t key = strtol(av[0], nullptr, 0);
  KeyswState state;
  switch(*av[1]) {
    case 'p':
      state = KS_PRESS;
      break;
    case 'P':
      state = KS_PARTIAL;
      break;
    case 't':
      state = KS_TAP;
      break;
    case 'h':
      state = KS_HOLD;
      break;
    case 'l':
      state = KS_HOLDL;
      break;
    case 'L':
      state = KS_HOLDLL;
      break;
    case 'v':
      state = KS_VALUE;
      break;
    default:
      state = KS_RELEASE;
  }
  KeymapLayerType layer = (KeymapLayerType)strtol(av[2], nullptr, 0);
  FunctionCode_t func = (FunctionCode_t)strtol(av[3], nullptr, 0);

  if(!ctxkeymap.keymap.setFunctionCode(key, state, layer, func)) {
    s->print("# setFunctionCode fail, invalid parameters\n");
    return(-1);
  }
  return(1);
}

const char *Utils::resetMCUEEPROMCmd = "reseteeprom";
const char *Utils::resetMCUEEPROMSyntax = "reseteeprom 0-5(MCU) | a(ll)";
const char *Utils::resetMCUEEPROMDescr = "Reset Press Calibration values on MCU EEPROM";
int Utils::resetMCUEEPROM(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  if(*av[0] == 'a') {
    KeySensorMCU::resetEEPROMAll();
    s->print("# All MCU EEPROM reset\n");
    return(1);
  }
  uint8_t mcu = strtol(av[0], nullptr, 0);
  if(mcu >= MCU_NMCUS) return -1;

  ctxmcu.mcu[mcu].resetEEPROM();
  s->print("# MCU ");
  s->print(mcu);
  s->print(" EEPROM reset\n");
  return(1);
}

const char *Utils::emergencyCmd = "emergency";
const char *Utils::emergencySyntax = "emergency 0(disable) | 1(enable)";
const char *Utils::emergencyDescr = "emergency mode control";
int Utils::emergency(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  switch(*av[0]) {
    case '0':
      ctxsys.kb_ctrl.enableEmergencyMode(0);
      s->print("# emergency mode disabled\n");
      break;
    case '1':
      ctxsys.kb_ctrl.enableEmergencyMode(1);
      s->print("# emergency mode enabled\n");
      break;
    default:
      return -1;
  }
  return(1);
}

const char *Utils::verCmd = "ver";
const char *Utils::verSyntax = "ver";
const char *Utils::verDescr = "get firmware version";
int Utils::ver(Stream *s, uint8_t ac, const char **av) {
  I2CVersionInfo *vinfo_p = (I2CVersionInfo *)setI2CVersionInfo(MF_MAJOR, MF_MINOR, MF_PATCH, 0);
  s->print("# Firmware version\n");
  s->print("Main : ");
  s->print(vinfo_p->ver_major);
  s->print('.');
  s->print(vinfo_p->ver_minor);
  s->print('.');
  s->print(vinfo_p->ver_patch);
  s->print(", ");
  s->print(vinfo_p->str);
  s->print('\n');

  I2CVersionInfo vinfo;
  for(uint8_t mcu = 0; mcu < MCU_NMCUS; mcu++) {
    s->print("MCU ");
    s->print(mcu);
    s->print(" : ");
    if(ctxmcu.mcu[mcu].getMCUVersion(vinfo) < 0) {
      s->print("error\n");
      continue;
    }
    vinfo.str[sizeof(vinfo.str)-1] = '\0';
    if(vinfo.mcu_num != mcu) {
      s->print("Number Mismatch ");
      s->print(vinfo.mcu_num);
      s->print(", ");
    }
    s->print(vinfo.ver_major);
    s->print('.');
    s->print(vinfo.ver_minor);
    s->print('.');
    s->print(vinfo.ver_patch);
    s->print(", ");
    s->print(vinfo.str);
    s->print('\n');
  }

  s->print("TP Left : ");
  if(ctxksm.tpsm_left.getVersion(vinfo) < 0) {
    s->print("not available\n");
  } else {
    if(vinfo.mcu_num != 0) {
      s->print("Number Mismatch ");
      s->print(vinfo.mcu_num);
      s->print(", ");
    }
    s->print(vinfo.ver_major);
    s->print('.');
    s->print(vinfo.ver_minor);
    s->print('.');
    s->print(vinfo.ver_patch);
    s->print(", ");
    s->print(vinfo.str);
    s->print('\n');
  }
  s->print("TP Right : ");
  if(ctxksm.tpsm_right.getVersion(vinfo) < 0) {
    s->print("not available\n");
  } else {
    if(vinfo.mcu_num != 1) {
      s->print("Number Mismatch ");
      s->print(vinfo.mcu_num);
      s->print(", ");
    }
    s->print(vinfo.ver_major);
    s->print('.');
    s->print(vinfo.ver_minor);
    s->print('.');
    s->print(vinfo.ver_patch);
    s->print(", ");
    s->print(vinfo.str);
    s->print('\n');
  }

  return(1);
}

#if defined(DEBUG)
const char *Utils::eventcliCmd = "event";
const char *Utils::eventcliSyntax = "event <txt1> <txt2>\n" \
  "  [pr(0:PR,1:ERR,2:INF) grp(0:NONE,1:CAL,2:MKERR) life dup(0:IGN,1:OVW,2:PROH)]\n" \
  "event : dump events";
const char *Utils::eventcliDescr = "set event or dump events";
int Utils::eventcli(Stream *s, uint8_t ac, const char **av) {
  if(ac == 0) {
    ctxsys.ev_handler.dumpSystemQ();
    return(1);
  }

  const char *txt1 = nullptr;
  const char *txt2 = nullptr;
  if(ac > 0) txt1 = (const char *)av[0];
  if(ac > 1) txt2 = (const char *)av[1];
  uint8_t pr = SystemEvent::PR_PRIORITY;
  uint8_t group = SystemEvent::GR_NONE;
  uint8_t dup = SystemEventQ::DUP_IGNORE;
  int8_t life = 1;
  if(ac > 5) {
    pr = strtol(av[2], nullptr, 0);
    if(pr >= SystemEvent::PR_MAX) pr = SystemEvent::PR_PRIORITY;
    group = strtol(av[3], nullptr, 0);
    if(group >= SystemEvent::GR_MAX) group = SystemEvent::GR_NONE;
    life = strtol(av[4], nullptr, 0);
    dup = strtol(av[5], nullptr, 0);
    if(dup >= SystemEventQ::DUP_MAX) dup = SystemEventQ::DUP_IGNORE;
  }
  ctxsys.ev_handler.pushNewEvent(pr, group, life, txt1, txt2, dup);
  return(1);
}

const char *Utils::debugCmd = "debug";
const char *Utils::debugSyntax = "debug";
const char *Utils::debugDescr = "Toggle on/off debug message";
int Utils::debug(Stream *s, uint8_t ac, const char **av) {
  ctxsys.kb_ctrl.enableDebugMode(ctxsys.kb_ctrl.isDebugMode() ? 0 : 1);
  s->print("# debug message ");
  s->print(ctxsys.kb_ctrl.isDebugMode() ? "on\n" : "off\n");
  return(1);
}

const char *Utils::slowCmd = "slow";
const char *Utils::slowSyntax = "slow [1-1000]";
const char *Utils::slowDescr = "On/Off slow mode, Off = no param, On = 1-1000 ms sleep in the loop()";
int Utils::slow(Stream *s, uint8_t ac, const char **av) {
  if(ac == 0) {
    s->print("# slow mode OFF\n");
    ctxsys.kb_ctrl.enableSlowMode(0);
  } else {
    s->print("# slow mode ON, delay ");
    s->print(ctxsys.kb_ctrl.enableSlowMode(strtol(av[0], nullptr, 0)));
    s->print(" ms\n");
  }
  return(1);
}

const char *Utils::hidrptCmd = "hidrpt";
const char *Utils::hidrptSyntax = "hidrpt k | m | t";
const char *Utils::hidrptDescr = "Toggle on/off keyboard/mouse/touchpad report";
int Utils::hidrpt(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  switch(*av[0]) {
    case 'k':
      ctxsys.kb_ctrl.enableKeyboardReport(ctxsys.kb_ctrl.isKeyboardReport() ? 0 : 1);
      if(!ctxsys.kb_ctrl.isKeyboardReport()) {
        ctxhid.hidmanager.clearSendReport();
      }
      break;

    case 'm':
      ctxsys.kb_ctrl.enableMouseReport(ctxsys.kb_ctrl.isMouseReport() ? 0 : 1);
      break;

    case 't':
      ctxsys.kb_ctrl.enableTouchpadReport(ctxsys.kb_ctrl.isTouchpadReport() ? 0 : 1);
      break;

    default:
      s->print("# invalid option\n");
      return -1;
  }

  s->print("# keyboard ");
  s->print(ctxsys.kb_ctrl.isKeyboardReport() ? "on" : "off");
  s->print(", mouse ");
  s->print(ctxsys.kb_ctrl.isMouseReport() ? "on" : "off");
  s->print(", touchpad ");
  s->print(ctxsys.kb_ctrl.isTouchpadReport() ? "on\n" : "off\n");
  return(1);
}

const char *Utils::tpparamCmd = "tpparam";
const char *Utils::tpparamSyntax = "tpparam l|r gain prescale oversamples rsel dzx*10 dzy*10\n" \
  "tpparam [l|r fd fmin beta] : fd is speed filter, smaller to smooth more. fmin is minimum cutoff, smaller to smooth more.";
const char *Utils::tpparamDescr = "Set/Get Touchpad parameters";
int Utils::tpparam(Stream *s, uint8_t ac, const char **av) {
#if defined(SUPPORT_TOUCHPAD)
  if(ac == 0) { // show one euro filter parameters
    oef_param_t oef;
    s->print("# Touchpad One Euro Filter Parameters\n");
    ctxreporter.f_touchpad_reporter.getOEFParam(TP_LEFT, oef);
    s->print("Left fd:");
    s->print(oef.fd);
    s->print(", fmin:");
    s->print(oef.fmin);
    s->print(", beta:");
    s->print(oef.beta);
    s->print('\n');
    ctxreporter.f_touchpad_reporter.getOEFParam(TP_RIGHT, oef);
    s->print("Right fd:");
    s->print(oef.fd);
    s->print(", fmin:");
    s->print(oef.fmin);
    s->print(", beta:");
    s->print(oef.beta);
    s->print('\n');
    return 1;
  }

  uint8_t mode = 0;
  switch(*av[0]) {
    case 'l':
      mode = 1;
      break;
    case 'r':
      mode = 2;
      break;
    default:
      return -1;
  }

  if(ac > 6) {
    uint8_t gain = strtol(av[1], nullptr, 0);
    uint8_t prescale = strtol(av[2], nullptr, 0);
    uint8_t oversamples = strtol(av[3], nullptr, 0);
    uint8_t rsel = strtol(av[4], nullptr, 0);
    uint16_t dzx = strtol(av[5], nullptr, 0);
    uint16_t dzy = strtol(av[6], nullptr, 0);
    bool reset = (ac > 7);
    if(mode == 1) {
      if(!ctxksm.tpsm_left.setParam(gain, prescale, oversamples, rsel, dzx, dzy, reset)) return -1;
      ctxrepl.repl.reserve("keyval l", 200);
    } else {
      if(!ctxksm.tpsm_right.setParam(gain, prescale, oversamples, rsel, dzx, dzy, reset)) return -1;
      ctxrepl.repl.reserve("keyval r", 200);
    }
  }

  else if(ac > 3) {
    oef_param_t oef = {
      .fd = strtof(av[1], nullptr),
      .fmin = strtof(av[2], nullptr),
      .beta = strtof(av[3], nullptr)
    };
    if(!ctxreporter.f_touchpad_reporter.setOEFParam((mode == 1 ? TP_LEFT : TP_RIGHT), oef)) {
      s->print("# Invalid OEF parameters. fd ");
      s->print(oef.fd);
      s->print(", fmin ");
      s->print(oef.fmin);
      s->print(", beta ");
      s->print(oef.beta);
      s->print('\n');
      return -1;
    }
  }

#else
  s->print("# Not supported in this board\n");
  return -1;
#endif

  return 1;
}

const char *Utils::getmsgCmd = "getmsg";
const char *Utils::getmsgSyntax = "getmsg 0-5(MCU)|l|r";
const char *Utils::getmsgDescr = "Get debug messages from MCU, Touchpad Left or Right";
int Utils::getmsg(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  uint8_t mcu = strtol(av[0], nullptr, 0);
  if(*av[0] != 'l' && *av[0] != 'r' && mcu >= MCU_NMCUS) return -1;

  if(*av[0] == 'l') {
#if defined(SUPPORT_TOUCHPAD)
    ctxksm.tpsm_left.showMsg(s);
#else
    s->print("# Not supported in this board\n");
#endif
  } else if(*av[0] == 'r') {
#if defined(SUPPORT_TOUCHPAD)
    ctxksm.tpsm_right.showMsg(s);
#else
    s->print("# Not supported in this board\n");
#endif
  } else {
    ctxmcu.mcu[mcu].showMCUMessage(s);
  }
  return(1);
}

const char *Utils::loopCmd = "loop";
const char *Utils::loopSyntax = "loop 0 | 1 | m(cu) 0-5";
const char *Utils::loopDescr = "Measure the execution time of loop(), loop1(), MCU loop()";
int Utils::loop(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  switch(*av[0]) {
    case '0':
      mainloop_showResults(s);
      break;

    case '1':
      ctxi2c.i2cloop.showResults(s);
      /*
      {
      fifomsg_t msg;
      msg.b[0] = I2CCtrl::LOOP1x100;
      rp2040.fifo.push(msg.i);
      uint32_t r = rp2040.fifo.pop();
      s->print("# loop1() x 100: ");
      s->print(r);
      s->print(" ms\n");
      }
      */
      break;

    case 'm':
      if(ac < 2) return -1;
      {
      uint8_t mcu = strtol(av[1], nullptr, 0);
      if(mcu >= MCU_NMCUS) return -1;
      multicoremsg_t msg;
      msg.cmd = I2CCtrl::GET_LOOPMEASURE;
      msg.addr = ctxmcu.mcu[mcu].getAddr();
      msg.buf = (uint8_t *)&mculoop;
      msg.bufsize = sizeof(mculoop);
      multicoreMsgPush(msg);
      multicoreMsgPop(msg);

      s->print("MCU ");
      s->print(mcu);
      s->print(" loop()\n");
      for(uint8_t i = 0; i < sizeof(mculoop.us)/sizeof(mculoop.us[0]); i++) {
        s->print(i);
        s->print(" : ");
        s->print(mculoop.us[i]);
        s->print(" us\n");
      }
      }
      break;

    default:
      s->print("# invalid option\n");
      return -1;
  }

  return 1;
}

const char *Utils::keytestCmd = "keytest";
const char *Utils::keytestSyntax = "keytest 0-41(key) <val> <interval(0-60000ms)>\nkeytest s(top)\n" \
  "  repeat key on(value)/off at interval. interval 0 to keep key on";
const char *Utils::keytestDescr = "key test of fake key value";
int Utils::keytest(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  multicoremsg_t msg;
  if(*av[0] == 's') {
    msg.cmd = I2CCtrl::SET_KEYTEST_STOP;
    ctxsys.kb_ctrl.enableKeytestMode(false);
  } else {
    if(ac < 3) return -1;
    uint8_t key = strtol(av[0], nullptr, 0);
    if(key >= DEFS_NMAGKEYS) return -1;
    uint8_t mcu = KeySensorMCU::keypos2mcu[key];
    keytestdata.key = KeySensorMCU::keypos2key[key];
    keytestdata.val = strtol(av[1], nullptr, 0);
    unsigned long interval = strtol(av[2], nullptr, 0);
    keytestdata.interval[0] = (interval & 0xFF00) >> 8;
    keytestdata.interval[1] = interval & 0xFF;
    s->print("# keytest : key ");
    s->print(key);
    s->print(" (MCU ");;
    s->print(mcu);
    s->print(", MCU key ");;
    s->print(keytestdata.key);
    s->print("), val ");
    s->print(keytestdata.val);
    s->print(", interval ");
    s->print(interval);
    s->print(" ms\n");
    msg.cmd = I2CCtrl::SET_KEYTEST_VAL;
    msg.addr = ctxmcu.mcu[mcu].getAddr();
    msg.buf = (uint8_t *)&keytestdata;
    msg.bufsize = sizeof(keytestdata);
    ctxsys.kb_ctrl.enableKeytestMode(true);
  }

  multicoreMsgPush(msg);
  multicoreMsgPop(msg);
  return(msg.result == 0 ? 1 : -1);
}

#endif

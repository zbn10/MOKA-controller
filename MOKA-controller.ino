#include <Arduino.h> 
#include <Wire.h>
#include "src/debuglog.h"
#include <DebugLog.h>
#include "src/controller.h"
#include "src/i2c_control.h"
#include "src/i2c.h"
#include "src/keysw_statemachine.h"
#include "src/custom_keymap.h"
#include "src/hid.h"
#include "src/config.h"
#include "src/mcu.h"
#include "src/repl.h"
#include "src/cliparser.h"
#include "src/utils.h"
#include "src/webhid.h"
#include "src/functions.h"
#include "src/sysutils.h"
#include "src/version.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
//#include <Adafruit_TinyUSB.h>






bool core1_separate_stack = true;
Config config;


FunctionHolder fholder;
WebHID webhid;


Cmdset cmdset[] = {
  Cmdset(CMDSET(Utils::calibration)),
  Cmdset(CMDSET(Utils::keyval)),
  Cmdset(CMDSET(Utils::led)),
  Cmdset(CMDSET(Config::km)),
  Cmdset(CMDSET(Utils::keycode)),
  Cmdset(CMDSET(CustomKeymap::macro)),
  Cmdset(CMDSET(Utils::keyattr)),
  Cmdset(CMDSET(Utils::tpparam)),

  Cmdset(CMDSET(Utils::sysstat)),
  Cmdset(CMDSET(Utils::ver)),
  Cmdset(CMDSET(Utils::emergency)),
  Cmdset(CMDSET(Utils::resetMCUEEPROM)),

#if defined(DEBUG)
  Cmdset(CMDSET(Utils::eventcli)),
  Cmdset(CMDSET(Utils::hidrpt)),
  Cmdset(CMDSET(Utils::debug)),
  Cmdset(CMDSET(Utils::slow)),
  Cmdset(CMDSET(Utils::getmsg)),
  Cmdset(CMDSET(Utils::loop)),
  Cmdset(CMDSET(Utils::keytest)),
#endif
};



void setup() {
  // off builtin LEDs
  disableBuiltinLED();

  ctxksm.tact_sw.begin(); // needed before Serial initialization because tact_sw PIN has been initialized
  delay(1);

  static uint8_t hidcfgbuf[2048];
  static uint8_t startup_tactsw_count = 0;
  if(ctxksm.tact_sw.isPressed()) { // Factory mode
    while(ctxksm.tact_sw.isPressed()) {
      delay(10);
      //if(startup_tactsw_count++ > 100) break;
    }
    ctxsys.kb_ctrl.enableFactoryMode(true);
  } else {
    ctxsys.kb_ctrl.enableFactoryMode(false);
  }


  if(ctxsys.kb_ctrl.isFactoryMode()) {
    ctxsys.kb_ctrl.enableEmergencyMode(true);
    ctxsys.kb_ctrl.enableKeyboardReport(true);
    ctxsys.kb_ctrl.enableMouseReport(false);
    ctxsys.kb_ctrl.enableTouchpadReport(true);
    ctxsys.kb_ctrl.enableDebugMode(true, false); // second parameter, MCU debug mode, is set later
    ctxsys.kb_ctrl.enableSlowMode(0);

    Serial.begin(DEFS_SERIALSPEED);
    LOG_ATTACH_SERIAL(Serial);
    ctxrepl.repl.setStream(&Serial);
    ctxi2c.debug_loop1.setStream(&Serial);
    ctxhid.debug_hidcallback.setStream(&Serial);
  } else {
    ctxsys.kb_ctrl.enableEmergencyMode(false);
    ctxsys.kb_ctrl.enableKeyboardReport(true);
    ctxsys.kb_ctrl.enableMouseReport(true);
    ctxsys.kb_ctrl.enableTouchpadReport(true);
    ctxsys.kb_ctrl.enableDebugMode(false, false); // second parameter, MCU debug mode, is set later
    ctxsys.kb_ctrl.enableSlowMode(0);

    TinyUSBDevice.clearConfiguration();
    Serial1.setRX(DEFS_PIN_SERIAL_RX);
    Serial1.setTX(DEFS_PIN_SERIAL_TX);
    Serial1.begin(DEFS_SERIALSPEED);
    LOG_ATTACH_SERIAL(Serial1);
    ctxrepl.repl.setStream(&Serial1);
    ctxi2c.debug_loop1.setStream(&Serial1);
    ctxhid.debug_hidcallback.setStream(&Serial1);
  }
  TinyUSBDevice.setConfigurationBuffer(hidcfgbuf, sizeof(hidcfgbuf));
  TinyUSBDevice.setManufacturerDescriptor(DEFS_USBDEVICE_MANUFACTURER);
  TinyUSBDevice.setProductDescriptor(DEFS_USBDEVICE_PRODUCTSTRING);


  ctxhid.hidmanager.begin();
  ctxhid.hidmanager.remount();
  if(ctxsys.kb_ctrl.isDebugMode()) { delay(3000); } // wait USB Serial console connection
  webhid.begin(); // set webhid buffer

  ctxrepl.repl.setCmdset(cmdset, sizeof(cmdset)/sizeof(Cmdset));
  CustomKeymap::setupFunctionInstances();

  rp2040.fifo.push(I2CCtrl::READY);
  uint32_t r = rp2040.fifo.pop();
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("FIFO pop: ", r);
  }
#endif
  delay(200);

  // set Resolution BEFORE ksm.begin()
  analogReadResolution(DEFS_ANALOGRESOLUTION);

  /*
    tactsw, photo_sensor, infrared_sensor need to set PinMode().
    TinyMouseStateMachine need to check availability and calibration.
  */
  for(uint8_t i = 0; i < DEFS_NKEYS; i++) {
    CustomKeymap::ksm[i]->begin();
  }

  /*
    SystemLED need NeoPixel initialize, OLED need SSD1306 initialize on I2C
  */
  Functions::beginAllInstances();
  KeySensorMCU::calibrationReleaseAll();
  
  /*
  If incorrect profile settings are interfering with operation,
  the factory mode should load the default settings.
  
  if(ctxsys.kb_ctrl.isFactoryMode()) {
    Config::loadConfiguration(PROFILE_DEFAULT);
  } else
  */
  {
    if(!Config::loadConfiguration(PROFILE_PRIMARY)) {
      LOG_ERROR("Failed to load primary profile. Loading default profile.");
      Config::loadConfiguration(PROFILE_DEFAULT);
    }
  }

  ctxsys.kb_ctrl.setDebugModeAll(); // apply debug mode to MCUs
  if(!checkVersionCompatibility()) {
    LOG_ERROR("Version compatibility check failed. Entering factory mode.");
    if(ctxsys.kb_ctrl.isFactoryMode()) {
      ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, -1, "Version Mismatch", "Fix it");
    } else {
      ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, -1, "Version Mismatch", "Use factory mode");
    }
  }

  if(ctxsys.kb_ctrl.isFactoryMode()) {
    ctxkeymap.keymap.setFunctionCode(CK_TACTSW, KS_HOLDL, LAYER_0, F_CHG_PROFD);
  }

  fholder.push(&ctxreporter.f_key_reporter);
  fholder.push(&ctxreporter.f_mouse_reporter);
  fholder.push(&ctxreporter.f_touchpad_reporter);
  fholder.push(&ctxkeymap.f_layer_handler);
  fholder.push(&ctxsys.ev_handler);
  //fholder.push(&ctxsys.f_systemled);
}


void keystate2function(FunctionHolder &fholder) {
  uint8_t start_key = 0, end_key = DEFS_NKEYS;
  KeymapLayerType layer = F_SelectLayer::getSelectedLayer();
  #if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("Selected layer ", layer);
  }
  #endif
  if(ctxsys.kb_ctrl.isEmergencyMode()) {
    // only tact switch is enabled in emergency mode
    start_key = CK_TACTSW;
    end_key = CK_TACTSW + 1;
    layer = LAYER_EMERGENCY;
  } else if(layer == LAYER_CALIBRATION) {
    start_key = DEFS_NMAGKEYS; // in calibration mode, skip mag keys
  }
  KeyswStateMachine::prepareTotalPressed(); // for advanced tap behavior

  // if in calibration mode, dont check 0-41 keys.
  for(uint8_t key = start_key; key < end_key; key++) {
    KeyswStateMachine *ksm = CustomKeymap::ksm[key];

    ksm->checkTransition(ctxkeymap.keymap.isTapAdvanced());
    KeyswState s = ksm->getState();
    FunctionCode_t fcode;
    Functions *f;
    switch(s) {
      case KS_RELEASE:
        ksm->storeFunctionCode(F_NONE);
        break;

      case KS_PARTIAL:
      case KS_PRESS:
      case KS_TAP:
      case KS_HOLD:
      case KS_HOLDL:
      case KS_HOLDLL:
        if(ksm->isPhysicalStateChanged()) { // partial, press
          ksm->storeTotalPressedInitial(KeyswStateMachine::getTotalPressedPreserved());
          fcode = ctxkeymap.keymap.getFunctionCode(key, s, layer);
          ksm->storeLayer((uint8_t)layer); // pin the layer
          ksm->storeFunctionCode(fcode); // pin the function code
          f = ctxkeymap.keymap.getFunctionInstance(fcode);
          f->setBeginning(true); // 'beginning' flag is used for one execution function.
          if(fcode != F_NONE) fholder.push(f, fcode);
#if defined(DEBUG)
          if(ctxsys.kb_ctrl.isDebugMode()) {
            LOG_DEBUG("key", key, ", state", s, ", fcode 0x", DebugLogBase::HEX, fcode, ", layer", layer,
              ", sChanged", ksm->isStateChanged(), ", PyChanged", ksm->isPhysicalStateChanged());
          }
#endif
        } else { // logical state changed, or not changed
          ksm->storeTotalPressedMax(KeyswStateMachine::getTotalPressedPreserved());
          if(ksm->isStateChanged()) { // logical state changed, ex. KS_PRESS -> KS_HOLD

            // if other keys are also pressed later, skip TAP function
            if(ctxkeymap.keymap.isTapAdvanced() && s == KS_TAP &&
              ksm->getStoredTotalPressedMax() > ksm->getStoredTotalPressedInitial() + 1)
            {
              ksm->storeFunctionCode(F_NONE);
              f = ctxkeymap.keymap.getFunctionInstance(F_NONE);
              f->setBeginning(false);
              //fholder.push(f);
#if defined(DEBUG)
              if(ctxsys.kb_ctrl.isDebugMode()) {
                LOG_DEBUG("key", key, ", state", s, "=> SKIPPED,  TotalPressedMax", ksm->getStoredTotalPressedMax(),
                  ", TotalPressedInitial", ksm->getStoredTotalPressedInitial());
              }
#endif
            }

            // normal KS_TAP or other state change
            else {
              KeymapLayerType stored_layer = (KeymapLayerType)ksm->getStoredLayer();
              FunctionCode_t stored_fcode = ksm->getStoredFunctionCode();
              fcode = ctxkeymap.keymap.getFunctionCode(key, s, stored_layer); // if keymap changed, stored_fcode may be invalid
              ksm->storeFunctionCode(fcode); // pin the function code
              f = ctxkeymap.keymap.getFunctionInstance(fcode);
              f->setBeginning(fcode != stored_fcode ? true : false);
              if(fcode != F_NONE) fholder.push(f, fcode);
#if defined(DEBUG)
              if(ctxsys.kb_ctrl.isDebugMode()) {
                LOG_DEBUG("key", key, ", state", s, ", fcode 0x", DebugLogBase::HEX, fcode, ", stored_layer", stored_layer,
                  ", sChanged", ksm->isStateChanged(), ", PyChanged", ksm->isPhysicalStateChanged());
              }
#endif
            }
          } else { // state NOT changed
            fcode = ksm->getStoredFunctionCode();
            f = ctxkeymap.keymap.getFunctionInstance(fcode);
            f->setBeginning(false);
            if(fcode != F_NONE) fholder.push(f, fcode);
          }
        }
        ksm->incTotalPressed();
        break;

      case KS_VALUE:
        // f_mouse etc have to be specified for each layer if needed.
        fcode = ctxkeymap.keymap.getFunctionCode(key, KS_VALUE, layer);
        f = ctxkeymap.keymap.getFunctionInstance(fcode);
        f->setInputValue32(ksm->getAnyValue());
        f->setBeginning(ksm->isStateChanged());
#if defined(DEBUG)
        if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
          LOG_DEBUG("key ", key, ", KS_VALUE", ksm->getAnyValue(), ", layer", layer, ", fcode 0x", DebugLogBase::HEX, fcode);
        }
#endif
        if(fcode != F_NONE) fholder.push(f, fcode);
        break;

      default:
        LOG_ERROR("Illegular ksm state ", s);
    }
  }
}

timeMeasurement mainloop;
void loop() {
  ctxsys.kb_ctrl.setPeriodicFlag();

#if defined(DEBUG)
  mainloop.measurement();
#endif

  ctxrepl.repl.eval();

  webhid_data_t webhid_buf;
  if(ctxhid.hidmanager.webhidReceiveRequest((uint8_t *)&webhid_buf, sizeof(webhid_buf))) {
    webhid.progress(webhid_buf);
  }
  
  keystate2function(fholder);
  //Functions::preserveLayer(F_SelectLayer::getSelectedLayer());
  F_SelectLayer::setSelectedLayer(LAYER_0);
  fholder.action(F_PR_HIGH); // immediate
  fholder.action(F_PR_MIDDLE); // normal key, layer selection
  fholder.action(F_PR_LOW); // hidsend, layer decision. f_key_hid handles TapAdvanced.

    // set key LED, check err state.
  if(!ctxsys.kb_ctrl.isCalibrationMode()) {
    KeySensorMCU::reflectKeyStateOnLED(CustomKeymap::ksm); // press state & fault state
  }

  //ctxsys.kb_ctrl.setTypeState(KeyboardControl::TYPE_MCU, mcuerr.hasError()); // handled in MCU::setError()
  //ctxsys.kb_ctrl.setTypeState(KeyboardControl::TYPE_KEY, keyerr.hasError()); // handled in I2C loop
  ctxsys.f_systemled.setKBLockState(
    ctxhid.hidmanager.isNumlock() || ctxsys.kb_ctrl.getPseudoState(LEDTYPE_NUMLOCK),
    ctxhid.hidmanager.isCapslock() || ctxsys.kb_ctrl.getPseudoState(LEDTYPE_CAPSLOCK),
    ctxhid.hidmanager.isScrolllock() || ctxsys.kb_ctrl.getPseudoState(LEDTYPE_SCROLLLOCK));

  ctxhid.error_hidcallback.show();
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.show();
    ctxi2c.debug_loop1.show();

    multicoremsg_t msg;
    msg.cmd = I2CCtrl::I2C_DMSG;
    multicoreMsgPush(msg);
    multicoreMsgPop(msg);
  }

  if(ctxsys.kb_ctrl.isSlowMode()) ctxsys.kb_ctrl.slowModeDelay();
#endif
}

void mainloop_showResults(Stream *s) {
  mainloop.showResults(s);
}

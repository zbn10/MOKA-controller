#pragma once
#include "i2c_control.h"
#include "controller.h"

class Functions;

#define KSM_CHATTERING_LIMIT 30 // key status changes per interval

  /* WebHID
  *** physical keymap, selected layer view
  *** per Key view
    PRESS, PARTIALPRESS, TAP, LONGPRESS1, LONGPRESS2, DOUBLECLICK
    supported layers vary by KSM type.
  */
enum KeyswState : uint8_t {
  KS_RELEASE = 0x00,
  KS_PARTIAL,
  KS_PRESSING, // internal state
  KS_PRESS, // -> HOLD -> HOLD2 -> HOLD3
  KS_TAP, // Release until HOLD
  KS_HOLD, // keep state while HOLD
  KS_HOLDL, // keep state while HOLDL
  KS_HOLDLL, // keep state while HOLDLL
  KS_RELEASING, // internal state
  KS_VALUE,
  KS_MAX,
};
#define validateKeyswState(state) ((state) < KS_MAX)

enum KeyswCtrl : uint8_t {
  KC_NONE = 0,
  KC_FORCE_OFF,
  KC_FORCE_ON,
  KC_MAX,
};

class KeyswConst {
public:
  static constexpr uint16_t DurPartialPress = 50; //ms
  static constexpr uint16_t DurHold = 900; //ms
  static constexpr uint16_t DurHoldL = 1000; //ms
  static constexpr uint16_t DurHoldLL = 3000; //ms
  static constexpr uint8_t ThrPartialPress = 60;
  static constexpr uint8_t ThrPress = 150;
  static constexpr uint8_t ThrPartial2Release = 30;
  static constexpr uint8_t ThrPress2Release = 50;
};

class KeyswAttributes { // profile save target
public:
  uint8_t thresh_partial_press = KeyswConst::ThrPartialPress;
  uint8_t thresh_press = KeyswConst::ThrPress;
  uint8_t thresh_partial2release = KeyswConst::ThrPartial2Release;
  uint8_t thresh_press2release = KeyswConst::ThrPress2Release;
  uint16_t duration_hold = KeyswConst::DurHold;
  KeyswCtrl control = KC_NONE; // uint8_t

  static constexpr uint16_t duration_partial_press = KeyswConst::DurPartialPress;
  static constexpr uint16_t duration_holdL = KeyswConst::DurHoldL;
  static constexpr uint16_t duration_holdLL = KeyswConst::DurHoldLL;
};

//template<typename T>
class KeyswStateMachine {
public:
  KeyswStateMachine() : keyval(0), state(KS_RELEASE), state_prev(KS_RELEASE) {}
  /*
   * setKeyval()
   * @param v key value, 0-255
   * @return actual key value
  */
  virtual void begin() = 0;
  virtual uint8_t setKeyval(uint8_t) = 0;
  virtual bool checkTransition(bool) = 0; // return true if transition occurred  
  virtual bool userDriven() = 0; // return true if the key is pressed by user action
  virtual const char *getClassName() = 0;

  /*
  virtual bool isRelease2PartialPress() = 0;
  virtual bool isRelease2Press() = 0;
  virtual bool isRelease2Tap() = 0;
  virtual bool isPress2Tap() = 0;
  virtual bool isPress2Hold() = 0;
  virtual bool isPress2HoldL() = 0;
  virtual bool isPress2HoldLL() = 0;
  virtual bool isPartialPress2Releasing() = 0;
  virtual bool isPress2Releasing() = 0;
  virtual bool isReleasing2Release() = 0;
  */

  bool getError() { return key_err; } // no use. if needed use KeySensorMCU
  void setError(uint8_t e) { key_err = e; }
  KeyswState getState() { return state; }
  KeyswState getStatePrev() { return state_prev; }
  const char *getActiveStateName();
  static const char *getStateName(KeyswState s);
  uint8_t getKeyval() { return keyval; }
  uint32_t getAnyValue() { return anyvalue; }
  bool isStateChanged() { return (state != state_prev); }
  bool isPhysicalStateChanged() { return(physical_state != physical_state_prev); }

  void storeFunctionCode(FunctionCode_t c) { stored_fcode = c; }
  FunctionCode_t getStoredFunctionCode() { return stored_fcode; }
  void storeLayer(uint8_t l) { stored_layer = l; }
  uint8_t getStoredLayer() { return stored_layer; }

  // private for inheritance classes
  FunctionCode_t stored_fcode;
  uint8_t stored_layer;
  uint8_t keyval;
  uint32_t anyvalue;
  KeyswState state;
  KeyswState state_prev; // previous state
  KeyswState physical_state;
  KeyswState physical_state_prev;
  unsigned long millis_press;
  unsigned long now;

  bool key_err = false;

  inline static uint8_t total_pressed = 0;
  inline static uint8_t total_pressed_preserved = 0;

  static void prepareTotalPressed() { total_pressed_preserved = total_pressed; total_pressed = 0; };
  static uint8_t getTotalPressedPreserved() { return total_pressed_preserved; }
  uint8_t total_pressed_initial_perinstance = 0, total_pressed_max_perinstance = 0;
  void incTotalPressed() { if(userDriven()) total_pressed++; };
  void storeTotalPressedInitial(uint8_t t) { total_pressed_initial_perinstance = t; total_pressed_max_perinstance = 0; }
  void storeTotalPressedMax(uint8_t t) { if(t > total_pressed_max_perinstance) total_pressed_max_perinstance = t; }
  uint8_t getStoredTotalPressedInitial() { return total_pressed_initial_perinstance; }
  uint8_t getStoredTotalPressedMax() { return total_pressed_max_perinstance; }
};

class PseudokeyStateMachine : public KeyswStateMachine {
public:
  PseudokeyStateMachine() : KeyswStateMachine() {}
  void begin() override {}
  uint8_t setKeyval(uint8_t v) override { return 0; }
  //bool checkTransition(bool x, bool y) override {
  bool checkTransition(bool tapadv) override {
    bool b = (state != state_prev);
    state_prev = state;
    physical_state_prev = physical_state;
    return b;
  }
  const char *getClassName()  override { return "Pseudokey"; }
  bool userDriven() override { return false; }

  bool setState(KeyswState s) { state = s; return true; }
};

class MagswStateMachine : public KeyswStateMachine {
public:
  MagswStateMachine(KeyswAttributes &attr)
    : KeyswStateMachine(), state_internal(KS_RELEASE), keyattr(attr) {};
  void begin() override {}
  uint8_t setKeyval(uint8_t) override;
  //bool checkTransition(bool, bool) override;
  bool checkTransition(bool) override;
  const char *getClassName()  override { return "Magsw"; }
  bool userDriven() override { return true; }

  bool isRelease2PartialPress();
  bool isRelease2Pressing();
  bool isRelease2Press();
  bool isPressing2Tap();
  bool isPressing2Press();
  bool isPress2Tap();
  bool isPress2Hold();
  bool isPress2HoldL();
  bool isPress2HoldLL();
  bool isPartialPress2Releasing();
  bool isPress2Releasing();
  bool isReleasing2Release();

  bool setKeyattr(uint8_t tpp, uint8_t tp, uint8_t tpp2r, uint8_t tp2r, uint16_t dur_hold);
  uint8_t getKeyattrPartialPress() { return keyattr.thresh_partial_press; }
  uint8_t getKeyattrPress() { return keyattr.thresh_press; }
  uint8_t getKeyattrPartial2Release() { return keyattr.thresh_partial2release; }
  uint8_t getKeyattrPress2Release() { return keyattr.thresh_press2release; }
  uint16_t getKeyattrDurationHold() { return keyattr.duration_hold; }
  void setCtrl(KeyswCtrl ctrl) { if(ctrl < KC_MAX) keyattr.control = ctrl; }
  bool getErrorMaskedbyForceOff() { return(key_err && keyattr.control != KC_FORCE_OFF); }
  bool getErrorOrForceOff() { return(key_err || keyattr.control == KC_FORCE_OFF); }
  bool getError() { return key_err; }
  static bool validateKeyattr(uint8_t tpp, uint8_t tp, uint8_t tpp2r, uint8_t tp2r, uint16_t dur_hold);
  static bool validateKeyattr(KeyswAttributes (&keyattr)[DEFS_NMAGKEYS]);
  static void setKeyErrorPresence(bool p) { keyerr_presence = p; }
  static bool getKeyErrorPresence() { return keyerr_presence; }

private:
  KeyswState state_internal = KS_RELEASE;
  unsigned long millis_partial = 0;
  unsigned long millis_partial_pressing = 0;
  bool partial_pressing = false;
  unsigned long millis_tap = 0;
  bool tapping = false;
  KeyswAttributes &keyattr;
  bool err_logged = false;
  inline static bool keyerr_presence = false;
};

class TactswStateMachine : public KeyswStateMachine {
public:
  TactswStateMachine(uint8_t p)
    : KeyswStateMachine(), pin(p), state_internal(KS_RELEASE) {}
  void begin() override {
    pinMode(pin, INPUT_PULLUP);
  }
  uint8_t setKeyval(uint8_t v) override { if(v) keyval = 255; else keyval = 0; return keyval; }
  bool checkTransition(bool) override;
  const char *getClassName()  override { return "Tactsw"; }
  bool userDriven() override { return true; }

  bool isRelease2Press();
  bool isPress2Hold() { return false; }
  bool isPress2HoldL();
  bool isPress2HoldLL();
  bool isPress2Releasing();
  bool isReleasing2Release() { return true; }

  bool swPressing();
  bool isPressed() { return(digitalRead(pin) == LOW); }

private:
  uint8_t pin;
  KeyswState state_internal;
  unsigned long lasttime = 0; // when previous check was done
  unsigned long millis_pressing = 0;
  bool pressing = false;
  unsigned long millis_releasing = 0;
  bool releasing = false;
  static constexpr uint8_t DurPressing = 30;
};

class PhotoICSensorStateMachine : public KeyswStateMachine {
public:
  enum Brightness : uint8_t {
    BR_DARK = 0,
    BR_MOONLIGHT,
    BR_DESKLIGHT,
    BR_BRIGHTROOM,
    BR_MAX,
  };
  enum SensorThresholds : int32_t {
    TH_DARK = 100,
    TH_DESKLIGHT = 500,
    TH_BRIGHTROOM = 1500,
  };

  PhotoICSensorStateMachine(uint8_t p) : KeyswStateMachine(), pin(p) {}
  void begin() override {}
  uint8_t setKeyval(uint8_t v) override { if(v) keyval = 255; else keyval = 0; return keyval; }
  bool checkTransition(bool) override;
  const char *getClassName()  override { return "PhotoIC"; }
  bool userDriven() override { return false; }

  int32_t getValue() { return value; }

private:
  uint8_t pin;
  int32_t value = 0;
  int32_t anyvalue_prev = 0;
  unsigned long last_transition_time = 0;
  unsigned long last_state_changed_time = 0;
};

class InfraredSensorStateMachine : public KeyswStateMachine {
public:
  InfraredSensorStateMachine(uint8_t p)
    : KeyswStateMachine(), pin(p) {}
  void begin() override {}
  uint8_t setKeyval(uint8_t v) override { if(v) keyval = 255; else keyval = 0; return keyval; }
  bool checkTransition(bool) override;
  const char *getClassName()  override { return "Infrared"; }
  bool userDriven() override { return false; }
  uint32_t getValue() { return value; }

private:
  uint8_t pin;
  uint32_t value = 0;
  unsigned long millis_lastexists = 0;
};


class TinyMouseStateMachine : public KeyswStateMachine {
public:
  TinyMouseStateMachine(uint8_t x_pin, uint8_t y_pin) : KeyswStateMachine(), x_pin(x_pin), y_pin(y_pin) {}
  void begin() override { if(checkAvailable()) { calibration(); } }
  uint8_t setKeyval(uint8_t v) override { return 0; } // no key value
  bool checkTransition(bool) override;
  const char *getClassName()  override { return "TinyMouse"; }
  bool userDriven() override { return false; }

  bool checkAvailable();
  bool isAvailable() { return available; };
  void calibration();
  //inline uint8_t getButton() { return button; };
  //inline bool getWheel() { return wheel; };
  //int8_t getXdelta();
  //int8_t getYdelta();
  //bool update(int8_t &x_delta, int8_t &y_delta, uint8_t &button);

private:
  bool available = false;
  uint8_t x_pin;
  uint8_t y_pin;

  struct params {
    int16_t center = 0;
    int16_t delta = 0;
    unsigned long micros_centered = 0;
    unsigned long micros_moved = 0;
    bool centered = true;
  } X, Y;

  bool checkTransitionAxis(struct params &axis, unsigned long now);
};


class TouchpadStateMachine : public KeyswStateMachine {
public:
  TouchpadStateMachine(uint8_t addr) : KeyswStateMachine(), i2c_addr(addr) {}
  void begin() override {}
  uint8_t setKeyval(uint8_t v) override { return 0; } // no key value
  bool checkTransition(bool) override;
  bool userDriven() override { return false; }
  const char *getClassName() override { return "Touchpad"; }

  uint8_t getAddr() { return i2c_addr; }
  void setAvailable(bool a) { available = a; }
  bool isAvailable() { return available; }
  bool getRawval();
  bool getParam();
  bool setParam(uint8_t gain, uint8_t prescale, uint8_t oversamples, uint8_t rsel, uint16_t dzx, uint16_t dzy, bool reset = false);
  int showVal(Stream *s);
  int showMsg(Stream *s);
  int getVersion(I2CVersionInfo &vinfo);

  I2CTPData tpdata;
  inline static I2CTPRawData i2cdata_rawval;
  inline static I2CTPParam i2cdata_param;
  inline static I2CMaxData i2cdata_msg;

private:
  uint8_t i2c_addr;
  bool available = false;
  unsigned long last_transition = 0;
  int16_t x_delta = 0, y_delta = 0;
  I2CTPData tpdata_prev = {};
};

typedef struct {
  TactswStateMachine &tact_sw;
#if defined(SUPPORT_TINYMOUSE)
  TinyMouseStateMachine &tmouse;
#endif
#if defined(SUPPORT_TOUCHPAD)
  TouchpadStateMachine &tpsm_left;
  TouchpadStateMachine &tpsm_right;
#endif
#if defined(SUPPORT_PHOTOIC_INFRARED)
  PhotoICSensorStateMachine &photoic_sw;
  InfraredSensorStateMachine &infrared_sw;
#endif
  PseudokeyStateMachine &dummy_sw;
} appcontext_ksm_t;
appcontext_ksm_t &appctxKsm();
inline appcontext_ksm_t ctxksm = appctxKsm();

typedef struct {
  KeyswAttributes (&magsw_attr_list)[DEFS_NMAGKEYS];
  MagswStateMachine (&msm)[DEFS_NMAGKEYS];
} appcontext_magsw_t;
appcontext_magsw_t &appctxMagsw();
inline appcontext_magsw_t &ctxmagsw = appctxMagsw();

#pragma once
#include <Arduino.h>
#include <vector>
#include <initializer_list>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "functions_code.h"

enum KeymapLayerType : uint8_t;

enum FunctionPriority : uint8_t {
  F_PR_LOW = 0,
  F_PR_MIDDLE,
  F_PR_HIGH,
  F_PR_MAX,
};
enum FunctionLifecycle : uint8_t {
  F_LC_ONCE = 0,
  F_LC_PERIOD, // processed for a certain period of time, like a Macro
  F_LC_CONTINUOUS,
};
enum ActionResult : uint8_t {
  F_RES_NONE = 0,
  F_RES_SUCCESS,
  F_RES_FAIL,
};

class Functions {
public:
  Functions(
    FunctionCode_t f, FunctionLifecycle s = F_LC_ONCE, FunctionPriority p = F_PR_MIDDLE)
    : fcode(f), lifecycle_origin(s), priority(p)
  {
    registerInstance();
  }

  virtual void begin() = 0;
  virtual ActionResult childaction(unsigned long ms, FunctionCode_t fc) = 0;
  virtual ActionResult action(unsigned long ms, FunctionCode_t fc) final {
    // common preprocessing can be used for all functions.
    // currently not used.
    return childaction(ms, fc);
  }
  void initLifecycle() { lifecycle = lifecycle_origin; }
  void setInputValue32(uint32_t value) { inputvalue32 = value; }
  uint32_t getInputValue32() { return inputvalue32; }
  FunctionCode_t getFunctionCode() { return fcode; }
  FunctionLifecycle getLifecycle() { return lifecycle; }
  FunctionPriority getPriority() { return priority; }
  //void duplicateCall() { duplicate_call = true; }
  void setBeginning(bool b) { beginning = b; }
  //static void preserveLayer(KeymapLayerType l) { layer = l; }

  unsigned long processed_time = 0;
  FunctionCode_t fcode;
  const FunctionLifecycle lifecycle_origin;
  FunctionLifecycle lifecycle;
  FunctionPriority priority;
  //uint8_t inputvalue;
  uint32_t inputvalue32;
  //bool duplicate_call = false;
  bool beginning = false; // used for judging the first call/press of function

  //inline static KeymapLayerType layer = LAYER_0;
  static void beginAllInstances();

private:
  static std::vector<Functions*> &getInstances() {
    static std::vector<Functions*> instances;
    return instances;
  }
  void registerInstance() {
    auto &i = getInstances();
    i.push_back(this);
  }
};


class F_ChangeProfile : public Functions {
public:
  F_ChangeProfile(FunctionCode_t f, uint8_t p) : Functions(f, F_LC_ONCE, F_PR_LOW), profile(p) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
private:
  uint8_t profile;
};

constexpr uint8_t MACRO_MAXENTRIES = 15;
constexpr uint8_t MACRO_MAXFUNCS = 10;
struct MacroEntry {
  unsigned long duration; // ms
  bool simultaneous;
  uint8_t num;
  FunctionCode_t fcode[MACRO_MAXFUNCS];
};
struct MacroSequence {
  uint8_t num;
  MacroEntry entry[MACRO_MAXENTRIES];
};

class F_KeyMacro : public Functions {
public:
  F_KeyMacro(FunctionCode_t f, MacroSequence &seq) : Functions(f, F_LC_PERIOD, F_PR_HIGH), seq(seq) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
  bool addSequence(unsigned long duration, bool simul, std::initializer_list<FunctionCode_t> fcodes);
  bool addSequence(unsigned long duration, bool simul, FunctionCode_t* fcodes, size_t count);
  //bool addSequence(uint8_t l, unsigned long start, FunctionCode_t code, unsigned long keep);
  uint8_t getEntries(MacroEntry **entry) { *entry = seq.entry; return(seq.num); }
  bool reset() {
    for(uint8_t s = 0; s < MACRO_MAXENTRIES; s++) {
      for(uint8_t i = 0; i < MACRO_MAXFUNCS; i++) {seq.entry[s].fcode[i] = 0;}
      seq.entry[s].num = 0;
    }
    seq.num = 0;
    return true;
  }

  static bool validate(MacroSequence &m);

private:
  bool running = false;
  MacroSequence &seq;
  uint8_t entry_index = 0;
  uint8_t code_index = 0;
};

class F_Key : public Functions {
public:
  F_Key(FunctionCode_t f, uint8_t code, uint8_t mod)
    : Functions(f, F_LC_ONCE, F_PR_MIDDLE), code(code), mod(mod) {}
  F_Key()
    : Functions(F_NONE, F_LC_CONTINUOUS, F_PR_LOW), code(0), mod(0) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
  uint8_t getCode() const { return code; }
  uint8_t getMod() const { return mod; }
  //static uint8_t *getHidKeycode() { return keycode_forevent; }
  //static uint8_t getHidModifier() { return modifier_forevent; }
  static void clearHidReport();
  static void setKeyboardProtocolFeature(bool);
  static bool toggleKeyboardProtocolFeature();

private:
  const uint8_t code;
  const uint8_t mod;
  inline static uint8_t keycode_forevent, modifier_forevent;
};

class F_Mouse : public Functions {
public:
  enum BTN : uint8_t {
    LEFT     = 1,
    RIGHT    = 2,
    MIDDLE   = 4,
    BACKWARD = 8,
    FORWARD  = 16,
  };
  union Movement {
    uint32_t value32;
    struct {
      int8_t x;
      int8_t y;
    };
  };

  F_Mouse(FunctionCode_t f) : Functions(f, F_LC_ONCE, F_PR_MIDDLE) {}
  F_Mouse() : Functions(F_NONE, F_LC_CONTINUOUS, F_PR_LOW) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
  static void clearHidReport();
private:
  bool stopped = false;
  void testCircle();

  // passing parameters to F_HIDMouse(F_MOUSE_MOVE)
  inline static uint8_t button = 0, button_prev = 0;
  inline static bool wheel = false;
  inline static int8_t xdelta = 0, ydelta = 0, xdelta_prev = 0, ydelta_prev = 0;
};

/*
  32bit anyValue
  MSB
    1bit : button
    1bit : touch
    2bit : unused
    12bit: x
    4bit : unused
    12bit: y
  LSB
*/
#define TP_GET_BTN(v) ((v) & (1<<31) ? 1 : 0)
#define TP_SET_BTN(v) ((v) ? (1<<31) : 0)
#define TP_GET_TOUCH(v) ((v) & (1<<30) ? 1 : 0)
#define TP_SET_TOUCH(v) ((v) ? (1<<30) : 0)
#define TP_GET_X(v) (((v)>>16) & 0x0fff)
#define TP_SET_X(v) (((v) & 0x0fff)<<16)
#define TP_GET_Y(v) ((v) & 0x0fff)
#define TP_SET_Y(v) ((v) & 0x0fff)
#define TP_STAT_RELEASE 0
#define TP_STAT_TOUCH 1 // the first touch
#define TP_STAT_TOUCHING 2 // continuous touch
#define TP_STAT_RELEASING 3
#define TP_LEFT 0
#define TP_RIGHT 1
#define TP_REPORT_INTERVAL_US 8000
#define TEST_Hz 1000

typedef struct {
  uint8_t touch_stat;
  bool touch;
  uint16_t x_input; // logical scale
  uint16_t y_input;
  uint16_t x_filtered; // logical scale
  uint16_t y_filtered;
  uint16_t x_filtered_prev;
  uint16_t y_filtered_prev;
  float angle_prev;
  float x_filtered_mm_prev;
  float y_filtered_mm_prev;
  float vx_prev;
  float vy_prev;
  uint8_t btn_stat;
  bool btn;
  uint8_t count;
} TPPoint;

typedef struct {
  float fd;
  float fmin;
  float beta;
} oef_param_t;

class F_Touchpad : public Functions {
public:

  F_Touchpad() : Functions(F_NONE, F_LC_CONTINUOUS, F_PR_LOW) {}
  F_Touchpad(FunctionCode_t f) :  Functions(f, F_LC_ONCE, F_PR_MIDDLE) {}

  void begin() override {}
  ActionResult childaction(unsigned long ms, FunctionCode_t fc) override;

  void oneEuroFilter(bool firsttouch, unsigned long diff_us, TPPoint &p, oef_param_t &oef);
  void getOEFParam(uint8_t tp, oef_param_t &param) { if(tp == TP_LEFT) param = oef_left; else param = oef_right; }
  bool setOEFParam(uint8_t tp, const oef_param_t &param) { if(!validateOEFParam(param)) return false; if(tp == TP_LEFT) oef_left = param; else oef_right = param; return true; }
  oef_param_t &getOEFParamRef(uint8_t tp) { return (tp == TP_LEFT ? oef_left : oef_right); }
  static bool validateOEFParam(const oef_param_t &param);
  void testCircle();
  void testVScroll();
  void testPinchZoom();

  static bool getMouseMode() { return mouse_mode; }

  static constexpr oef_param_t oef_default = {
    .fd = 0.5f, // cutoff for acceleration, small: more smoothing, large: higher tracking ability. standard is 1.0Hz?
    .fmin = 1.5f, // minimum cutoff, decrease to reduce jitter when slow, increase to increase tracking ability when slow.
    .beta = 0.01f, // speed coefficient, 0.002 ~ 0.08 s/mm, high tracking ability when fast
  };

private:
  static constexpr float conversion_factor = 160.0; // logical unit 3200 to 20 mm
  inline static bool mouse_mode = false;
  inline static oef_param_t oef_left = oef_default;
  inline static oef_param_t oef_right = oef_default;

  uint16_t circle = 0;
  unsigned long circle_last_time = 0;
  bool toggle = true;

  inline static TPPoint left = {TP_STAT_RELEASE}, right = {TP_STAT_RELEASE}, scroll = {TP_STAT_RELEASE};
  TPPoint vpad = {TP_STAT_RELEASE};
};

class F_None : public Functions {
public:
  F_None() : Functions(F_NONE, F_LC_ONCE, F_PR_LOW) {}
  void begin() override {}
  ActionResult childaction(unsigned long ms, FunctionCode_t fc) override { return F_RES_NONE; }
};

class F_TapAdvanced : public Functions {
public:
  F_TapAdvanced()
    : Functions(F_TAPADVANCED, F_LC_ONCE, F_PR_LOW) {}
  void begin() override {}
  ActionResult childaction(unsigned long ms, FunctionCode_t fc) override;
};

class FunctionHolder {
public:
  FunctionHolder() {}
  bool push(Functions *f, FunctionCode_t fc = F_NONE);
  //bool remove(Functions *f);
  void action(FunctionPriority priority);
private:
  static constexpr uint8_t MAX_QUEUE_SIZE = 50;
  Functions *list[F_PR_MAX][MAX_QUEUE_SIZE];
  FunctionCode_t fcode[F_PR_MAX][MAX_QUEUE_SIZE];
  uint8_t count[F_PR_MAX] = {};
  bool remove(uint8_t priority, uint8_t index);
};

typedef struct {
  F_Key f_key_reporter;
  F_Mouse f_mouse_reporter;
  F_Touchpad f_touchpad_reporter;
} appcontext_reporter_t;
appcontext_reporter_t &appctxReporter();
inline appcontext_reporter_t &ctxreporter = appctxReporter();

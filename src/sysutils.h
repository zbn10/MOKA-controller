#pragma once
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#include "controller.h"
#include "functions.h"
#include "mcu.h"
#include "utils.h"


class ConfigKeyboardLED;
class I2CLEDAttributes;


class LEDConst {
public:
  static constexpr uint8_t LED_PTN_RELEASE = I2CCtrl::LED_PTN_RAINBOW;
  static constexpr uint8_t LED_PTN_PARTIALPRESS = I2CCtrl::LED_PTN_YELLOW;
  static constexpr uint8_t LED_PTN_PRESS = I2CCtrl::LED_PTN_BLUE;
  static constexpr uint8_t LED_PTN_ALERT = I2CCtrl::LED_PTN_RED;
  static constexpr uint8_t LED_BACKLIGHT = 10;
  static constexpr uint8_t LED_BRIGHTNESS = 100;

  inline static const uint32_t LED_COLOR_CALIBRATION = Adafruit_NeoPixel::Color(255, 255, 255); // White
  inline static const uint32_t LED_COLOR_SYSTEM = Adafruit_NeoPixel::Color(0, 0, 255); // Blue
  inline static const uint32_t LED_COLOR_MCUERR = Adafruit_NeoPixel::Color(255, 0, 0); // Red
  inline static const uint32_t LED_COLOR_KEYERR = Adafruit_NeoPixel::Color(255, 255, 0); // Yellow
  inline static const uint32_t LED_COLOR_NUMLOCK = Adafruit_NeoPixel::Color(0, 255, 255); // Skyblue
  inline static const uint32_t LED_COLOR_CAPSLOCK = Adafruit_NeoPixel::Color(0, 255, 0); // Green
  inline static const uint32_t LED_COLOR_SCROLLLOCK = Adafruit_NeoPixel::Color(255, 0, 255); // Purple
  //Adafruit_NeoPixel::Color(0, 0, 255)  // Blue
};


class CircularBuffer : public Stream {
  public:
    CircularBuffer() {}
    void setStream(Stream *s) {
      const_cast<CircularBuffer*>(this)->stream = s;
    }

    int available() override { return(len > 0); }
    int read() override { return(0); }
    int peek() override { return(0); }
    void flush() override {}

    void show() {
      if(stream == nullptr) {
        len = 0;
        start = 0;
        return;
      }

      while(len) {
        stream->print((char)buf[start++]);
        if(start >= sizeof(buf)) start = 0;
        len--;
      }
    }

    uint8_t copy(uint8_t *dst, uint8_t dstlen) {
      uint8_t p_dst = 0;
      while(p_dst < dstlen && len) {
        dst[p_dst++] = buf[start++];
        if(start >= sizeof(buf)) start = 0;
        len--;
      }
      return(p_dst);
    }

    size_t write(uint8_t b) override {
      uint16_t pos = start + len;
      if(pos >= sizeof(buf)) pos -= sizeof(buf);
      buf[pos] = b;
      len++;
      if(len > sizeof(buf)) {
        start++;
        if(start >= sizeof(buf)) start = 0;
        len = sizeof(buf);
      }
      return(1);
    }

  private:
    Stream *stream = nullptr;
    uint8_t buf[1000];
    uint16_t start = 0;
    uint16_t len = 0;
};


#define APPEND_STR(buf, pos, ...) do { \
  int __n = snprintf((buf) + (pos), sizeof(buf) - (pos), __VA_ARGS__); \
  if (__n > 0) (pos) += __n; \
  } while (0)

class SystemEvent {
public:
  enum : uint8_t {
    PR_PRIORITY = 0,
    PR_ERROR,
    PR_INFO,
    PR_DEBUG,
    PR_MAX,
  };
  enum : uint8_t { // groups for errors with duplicates
    GR_NONE = 0,
    GR_CALIBRATION,
    GR_MCUKEYERR,
    GR_LAYERHOLD,
    GR_MAX,
  };
  SystemEvent() : next(nullptr), prev(nullptr), life(0), func1(nullptr), func2(nullptr) {
    msg1[0] = '\0'; msg2[0] = '\0';
  }
  void setEvent(uint8_t pr, uint8_t gr, int8_t l, uint8_t t, const char *m1, const char *m2, const char *(*f1)(void), const char *(*f2)(void));
  void setID(uint8_t i) { id = i; }
  uint8_t getID() { return id; }
  void setPriority(uint8_t p) { priority = p; }
  uint8_t getPriority() { return priority; }
  uint8_t getGroup() { return group; }
  uint8_t getType() { return type; }
  int8_t getLife() { return life; }
  int8_t updateLife() { return (life < 0 ? life : (life > 0 ? --life : 0)); }
  void setNext(SystemEvent *n) { next = n; }
  void setPrev(SystemEvent *p) { prev = p; }
  SystemEvent *getNext() { return next; }
  SystemEvent *getPrev() { return prev; }
  const char *getMsg1() { return (msg1[0] == '\0' ? nullptr : msg1); }
  const char *getMsg2() { return (msg2[0] == '\0' ? nullptr : msg2); }
  const char *execFunc1() { return (func1 != nullptr ? func1() : nullptr); }
  const char *execFunc2() { return (func2 != nullptr ? func2() : nullptr); }

  static constexpr uint8_t TYPE_TEXT = 1;
  static constexpr uint8_t TYPE_FUNC = 2;

private:
  SystemEvent *prev;
  SystemEvent *next;
  uint8_t id;
  uint8_t priority;
  uint8_t group;
  uint8_t type;
  int8_t life; // <0: infinite, 0: end life, >0: alive
  char msg1[32];
  char msg2[32];
  const char *(*func1)(void);
  const char *(*func2)(void);
};

class SystemEventQ {
public:
  enum : uint8_t {
    DUP_IGNORE = 0, // allow duplicates
    DUP_OVERWRITE, // overwrite existing event
    DUP_PROHIBIT, // prohibit duplicates, first to win
    DUP_MAX,
  };

  SystemEventQ() : head(nullptr), tail(nullptr), count(0) {}

  void initQ(SystemEvent *events_array, uint8_t size);
  SystemEvent *pushBack(SystemEvent *ev, uint8_t check_duplicate = DUP_IGNORE);
  SystemEvent *popFront();
  uint8_t getCount() { return count; }

private:
  SystemEvent *head;
  SystemEvent *tail;
  uint8_t count;
};

// for F_SystemLED, KeyboardControl
enum : uint8_t {
  LEDTYPE_CALIBRATION = 0,
  LEDTYPE_SYSTEM,
  LEDTYPE_MCU,
  LEDTYPE_KEY,
  LEDTYPE_NUMLOCK,
  LEDTYPE_CAPSLOCK,
  LEDTYPE_SCROLLLOCK,
  LEDTYPE_MAX,

  KEYLED_RELEASE,
  KEYLED_PRESS,
  KEYLED_PARTIALPRESS,
  KEYLED_ALERT,
};

// F_SystemLED inherits Functions to initialize in Functions::beginAllInstances()
class F_SystemLED : public Adafruit_NeoPixel, public Functions {
public:
  F_SystemLED(uint8_t p, ConfigKeyboardLED& k, I2CLEDAttributes &ka) :
#if defined(SUPPORT_LOCKLED)
    Adafruit_NeoPixel(4, p, DEFS_SYS_RGB_TYPE + NEO_KHZ800),
#else
    Adafruit_NeoPixel(1, p, DEFS_SYS_RGB_TYPE + NEO_KHZ800),
#endif
    Functions(F_NONE, F_LC_ONCE, F_PR_LOW), kbled(k), keyled(ka) {}

  void begin() override;
  ActionResult childaction(unsigned long ms, FunctionCode_t fc) override;
  void display(unsigned long ms);

  void setSystemState(bool calibration, bool system, bool mcu, bool key) {
    type_state[LEDTYPE_CALIBRATION] = calibration;
    type_state[LEDTYPE_SYSTEM] = system;
    type_state[LEDTYPE_MCU] = mcu;
    type_state[LEDTYPE_KEY] = key;
  }
  void setKBLockState(bool numlock, bool capslock, bool scrolllock) {
    type_state[LEDTYPE_NUMLOCK] = numlock;
    type_state[LEDTYPE_CAPSLOCK] = capslock;
    type_state[LEDTYPE_SCROLLLOCK] = scrolllock;
  }
  void setLEDColor(uint8_t type, uint8_t r, uint8_t g, uint8_t b);
  void setKeyLEDColor(uint8_t type, uint8_t ptn);
  void applyBrightness(uint8_t brightness);
  void applyBacklight(uint8_t backlight);
  bool isBacklightEnabled() { return keyled.backlight_enable; }
  void enableBacklight(bool e);
  void applyLEDSettings() { applyBrightness(keyled.led_brightness); } // apply all LED settings. applyBrightness() totally do this.

  //void applyMCULEDs();
  static uint8_t getLEDColorR(ConfigKeyboardLED &ckled, uint8_t type);
  static uint8_t getLEDColorG(ConfigKeyboardLED &ckled, uint8_t type);
  static uint8_t getLEDColorB(ConfigKeyboardLED &ckled, uint8_t type);
  static bool validateLEDAttr(I2CLEDAttributes &ledattr);

private:
#if defined(SUPPORT_LOCKLED)
  static constexpr uint8_t LEDTYPE_SYSMAX = LEDTYPE_NUMLOCK;
#else
  static constexpr uint8_t LEDTYPE_SYSMAX = LEDTYPE_MAX;
#endif

  bool type_state[LEDTYPE_MAX] = {};
  ConfigKeyboardLED &kbled;
  I2CLEDAttributes &keyled;
  uint8_t backlight_backup;

  uint8_t type_active = 0;
  bool sysblink = true;

#if defined(SUPPORT_LOCKLED)
  uint32_t led_color_prev[4] = {};
#else
  uint32_t led_color_prev[1] = {};
#endif

  static constexpr uint8_t LEDPOSITION_SYS = 0;
#if defined(SUPPORT_LOCKLED)
  static constexpr uint8_t LEDPOSITION_NUMLOCK = 1;
  static constexpr uint8_t LEDPOSITION_CAPSLOCK = 2;
  static constexpr uint8_t LEDPOSITION_SCROLLLOCK = 3;
#endif
};


class F_EventHandler : public Functions {
public:
  F_EventHandler() : Functions(F_NONE, F_LC_CONTINUOUS, F_PR_LOW) {
    empty_q.initQ(events, sizeof(events)/sizeof(events[0]));
    initSystemQ();
  };
  F_EventHandler(FunctionCode_t fcode) : Functions(fcode, F_LC_ONCE, F_PR_MIDDLE) {};
  void begin() override {}
  ActionResult childaction(unsigned long ms, FunctionCode_t fc) override;

  bool pushNewEvent(uint8_t pr, uint8_t gr, int8_t life, const char *msg1, const char *msg2, uint8_t check_duplicate = SystemEventQ::DUP_IGNORE);
  bool pushNewEvent(uint8_t pr, uint8_t gr, int8_t life, const char *(*func1)(void), const char *(*func2)(void), uint8_t check_duplicate = SystemEventQ::DUP_IGNORE);
  void setKeycode(uint8_t k, uint8_t m) { keycode = k; modifier = m; }
  void dumpSystemQ();

private:
  SystemEvent events[20];
  /*
    Debug event is set to info_debug_q.
    The use of debug event is handled in childaction().
  */
  SystemEventQ empty_q, priority_q, error_q, info_debug_q, dummy_q;
  static constexpr uint16_t TIMER_LONG = 10000; // ms
  static constexpr uint16_t TIMER_SHORT = 5000; // ms
  unsigned long event_timer = TIMER_LONG;
  static constexpr unsigned long sub_event_timer = 1000;
  unsigned long sub_processed_time = 0;
  void initSystemQ();

  SystemEvent *current_event = nullptr;
  bool pushNewEventInternal(SystemEvent *ev, uint8_t check_duplicate = SystemEventQ::DUP_IGNORE); // push ev to Q
  bool updateLifeOfCurrentEvent(bool force_transition = false);
  SystemEventQ &getEventQByPriority(uint8_t pr) {
    return(
      pr == SystemEvent::PR_PRIORITY ? priority_q :
      pr == SystemEvent::PR_ERROR ? error_q :
      pr == SystemEvent::PR_INFO ? info_debug_q :
      pr == SystemEvent::PR_DEBUG ? info_debug_q :
      dummy_q // should not happen
    );
  }

  ActionResult drawOLED(SystemEvent *ev);
  inline static uint8_t keycode; // for system event
  inline static uint8_t modifier; // for system event
  static const char *getKeyCode();
  static const char *getKeyMod();
  static const char *getKeymapProfile();
  static const char *getKeymapOSLayout();
  static const char *getKbdMode();
  static const char *getTPMode();
#if defined(SUPPORT_PHOTOIC_INFRARED)
  inline static char photoic_buf[32];
  inline static char infrared_buf[32];
  static const char *getPhotoIC();
  static const char *getInfrared();
#endif
  inline static bool chg_event;
};


class KeyboardControl {
public:
  //KeyboardControl(bool USB_console, bool debug_message, bool keyboard_report)
  //: debug_message(debug_message), keyboard_report(keyboard_report), USB_console(USB_console) {}
  KeyboardControl() {}
  static KeyboardControl &getInstance() {
    static KeyboardControl instance;
    return(instance);
  }

  bool enableFactoryMode(bool b) { factory_mode = b; return b; }
  bool isFactoryMode() { return factory_mode; }
  bool enableEmergencyMode(bool b) { emergency_mode = b; return b; }
  bool isEmergencyMode() { return emergency_mode; }
  bool isPeriod() { return flag_period; }
  bool enableCalibrationMode(bool b) { flag_calilbration = b; if(!b) flag_exitedcalibration = true; return b; }
  bool isCalibrationMode() { return flag_calilbration; }
  bool exitedCalibrationMode() { if(flag_exitedcalibration) {flag_exitedcalibration = false; return(true); } else return(false); }
  void setPeriodicFlag() {
    unsigned long millis_now = millis();
    if(millis_now - millis_for_action_prev >= DEFS_PERIODIC_INTERVAL) {
      flag_period = true;
      millis_for_action_prev = millis_now;
    } else {
      flag_period = false;
    }
  }

#if defined(DEBUG)
  void setDebugModeAll();
  bool enableDebugMode(bool b, bool setmcu = true) {
    debug = b;
    if(setmcu) setDebugModeAll();
    return debug;
  };
  bool isDebugMode() { return debug; }
  //bool enablePeriodicDebugMode(bool b) { debug_periodic = b; return b; }
  //bool isDebugPeriodic() { return (debug_periodic && flag_period); }
  //bool isPeriodicDebugMode() { return debug_periodic; }
  uint16_t enableSlowMode(uint16_t t) { slow_delay_ms = (t < 1000 ? t : 1000); return slow_delay_ms; }
  bool isSlowMode() { return(slow_delay_ms > 0); }
  void slowModeDelay() { if(slow_delay_ms > 0) delay(slow_delay_ms); }
  uint16_t getSlowModeDelay() { return slow_delay_ms; }
  bool enableKeyboardReport(bool b) { keyboard_report = b; return b; }
  bool isKeyboardReport() { return keyboard_report; }
  bool enableMouseReport(bool b) { mouse_report = b; return b; }
  bool isMouseReport() { return mouse_report; }
  bool enableTouchpadReport(bool b) { touchpad_report = b; return b; }
  bool isTouchpadReport() { return touchpad_report; }
  //void enableTapAdvanced(bool b) { tap_behavior = b; }
  //bool isTapAdvanced() { return tap_behavior; }
  void enableKeytestMode(bool b) { keytest_mode = b; }
  bool isKeytestMode() { return keytest_mode; }

  void enablePseudoSysFault(bool f) { pseudo_state[LEDTYPE_CALIBRATION] = f; }
  bool isPseudoSysFault() { return pseudo_state[LEDTYPE_CALIBRATION]; }
  void enablePseudoMCUFault(bool f) { pseudo_state[LEDTYPE_MCU] = f; }
  bool isPseudoMCUFault() { return pseudo_state[LEDTYPE_MCU]; }
  void enablePseudoKeyFault(bool f) { pseudo_state[LEDTYPE_KEY] = f; }
  bool isPseudoKeyFault() { return pseudo_state[LEDTYPE_KEY]; }
  void enablePseudoNumlock(bool f) { pseudo_state[LEDTYPE_NUMLOCK] = f; }
  bool isPseudoNumlock() { return pseudo_state[LEDTYPE_NUMLOCK]; }
  void enablePseudoCapslock(bool f) { pseudo_state[LEDTYPE_CAPSLOCK] = f; }
  bool isPseudoCapslock() { return pseudo_state[LEDTYPE_CAPSLOCK]; }
  void enablePseudoScrolllock(bool f) { pseudo_state[LEDTYPE_SCROLLLOCK] = f; }
  bool isPseudoScrolllock() { return pseudo_state[LEDTYPE_SCROLLLOCK]; }
  bool getPseudoState(uint8_t type) { return (type < LEDTYPE_MAX ? pseudo_state[type] : false); }
#endif
  void enableSSD1306(bool b) { SSD1306_enable = b; }
  bool isSSD1306() { return SSD1306_enable; }

private:
  /*
  factory mode functions.
    enable USB console.
    use factory default settings except saved settings.
  */
  bool factory_mode = false;
  bool emergency_mode = false;
  bool SSD1306_enable = false;
  unsigned long millis_for_action_prev = 0;
  bool flag_period = false;
  bool flag_calilbration = false;
  bool flag_exitedcalibration = false;
  //bool tap_behavior = true; // false: normal behavior, true: ignore tap if pressed with other key.
  bool keytest_mode = false;

#if defined(DEBUG)
  /*
  on/off logging debug message except ERROR.
  debug messaging sometimes affects real-time behavior because it takes long time to display msgs.
  */
  bool debug = true;
  bool debug_periodic = true;
  uint16_t slow_delay_ms = 0;

  /*
  disable hid report if it gets in the way of debugging.
  */
  bool keyboard_report = true;
  bool mouse_report = true;
  bool touchpad_report = true;
  bool pseudo_state[LEDTYPE_MAX] = {};
#endif
};


//class OLED : public Adafruit_SSD1306, public Functions {
class OLED : public Adafruit_SSD1306 {
public:
  static const uint16_t STRLEN = 128;
  enum : uint8_t {
    CMD_DISPLAY = 0x01,
    CMD_SCROLLSTART,
    CMD_SCROLLSTOP,
    CMD_INVERTDISPLAY,
    CMD_DIM,
  };
  enum : uint8_t {
    PTN_UNDO = 0,
    PTN_CALIBRATION,
    PTN_LOGO1,
    PTN_KEY,
#if defined(SUPPORT_PHOTOIC_INFRARED)
    PTN_SENSOR,
#endif
    PTN_NONE,
    PTN_CEILING,
  };
  OLED(FunctionCode_t f, uint8_t w, uint8_t h, TwoWire *twi, int8_t rstpin, uint32_t clkduring)
    : Adafruit_SSD1306(w, h, twi, rstpin, clkduring) {}
  void begin(uint8_t addr);
  void drawText(const char *, const char *);
  void drawPtn() { drawPtn(ptn); }
  void drawPtn(uint8_t p);
  uint8_t nextPtn();

private:
  uint8_t ptn = PTN_LOGO1;
  uint8_t ptn_hold = 0;
  char str[STRLEN+1] = {};
  unsigned long timer = 10000; // ms
  uint8_t cnt = 0;
};



class F_KeyboardReset : public Functions {
public:
  F_KeyboardReset(FunctionCode_t f) : Functions(f, F_LC_ONCE, F_PR_HIGH) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
private:
};


/*
typedef union {
  uint8_t b[4];
  uint32_t i;
} fifomsg_t;
 */

typedef union {
  struct {
    union {
      uint8_t *buf; // 4 (arduino rp2040)
      uint8_t led_state[2];
      uint8_t calibration_key;
      uint8_t debug_mode;
      uint8_t oled_cmd;
      uint8_t ui8[4];
    };
    uint8_t bufsize; // 1
    uint8_t cmd; // 1
    uint8_t addr; // 1
    uint8_t result;
  };
  uint32_t ui32[2];
} multicoremsg_t;
static_assert(sizeof(multicoremsg_t) == 8, "multicoremsg_t size error");

inline bool multicoreMsgPop(multicoremsg_t &msg, bool nb = false) {
  if(nb) {
    if(rp2040.fifo.pop_nb(&(msg.ui32[0]))) {
      msg.ui32[1] = rp2040.fifo.pop();
      return true;
    }
  } else {
    msg.ui32[0] = rp2040.fifo.pop();
    msg.ui32[1] = rp2040.fifo.pop();
    return true;
  }

  return false;
}

inline void multicoreMsgPush(multicoremsg_t &msg) {
  rp2040.fifo.push(msg.ui32[0]);
  rp2040.fifo.push(msg.ui32[1]);
}

typedef struct {
  KeyboardControl &kb_ctrl;
  F_EventHandler &ev_handler;
  F_SystemLED &f_systemled;
  OLED &oled;
} appcontext_sys_t;
appcontext_sys_t &appctxSys();

inline appcontext_sys_t &ctxsys = appctxSys();

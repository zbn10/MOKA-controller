#pragma once

#define isTP(addr) ((addr) == I2CCtrl::TPL_ADDR || (addr) == I2CCtrl::TPR_ADDR)

class I2CCtrl {
public:
  // MCUD for stub MCUs
  static constexpr uint8_t MCU0_ADDR = 0x08;
  static constexpr uint8_t MCU1_ADDR = 0x09;
  static constexpr uint8_t MCU2_ADDR = 0x0A;
  static constexpr uint8_t MCU3_ADDR = 0x0B;
  static constexpr uint8_t MCU4_ADDR = 0x0C;
  static constexpr uint8_t MCU5_ADDR = 0x0D;
  static constexpr uint8_t MCU0D_ADDR = 0x10;
  static constexpr uint8_t MCU1D_ADDR = 0x20;
  static constexpr uint8_t MCU2D_ADDR = 0x30;
  static constexpr uint8_t MCU3D_ADDR = 0x40;
  static constexpr uint8_t MCU4D_ADDR = 0x50;
  static constexpr uint8_t MCU5D_ADDR = 0x60;
  static constexpr uint8_t MCUD_MASK = 0x70;

  // I2C 2nd channel
  static constexpr uint8_t SSD1306_ADDR = 0x3C;
  static constexpr uint8_t TPL_ADDR = 0x0E; // TrackPad Left
  static constexpr uint8_t TPR_ADDR = 0x0F; // TrackPad Right

  static constexpr uint8_t GET_VAL = 0x01;
  static constexpr uint8_t GET_RAWVAL = 0x02;
  static constexpr uint8_t GET_RAWRELEASE = 0x03;
  static constexpr uint8_t GET_RAWPRESS = 0x04;
  static constexpr uint8_t GET_LOOPMEASURE = 0x05;
  static constexpr uint8_t GET_MSG = 0x06;

  static constexpr uint8_t SET_VAL = 0x10; // unused yet
  static constexpr uint8_t CALIBRATION_RELEASE = 0x11;
  static constexpr uint8_t CALIBRATION_PRESS = 0x12;
  static constexpr uint8_t CALIBRATION_MODE_ENTER = 0x13;
  static constexpr uint8_t CALIBRATION_MODE_EXIT = 0x14;
  static constexpr uint8_t SET_KEYTEST_VAL = 0x15;
  static constexpr uint8_t SET_KEYTEST_STOP = 0x16;

  static constexpr uint8_t SET_BACKLIGHT = 0x21; // unused yet
  static constexpr uint8_t SET_LED_BRIGHTNESS = 0x22; // unused yet
  static constexpr uint8_t SET_LED_ATTRIBUTES = 0x23;
  static constexpr uint8_t SET_LED_STATE = 0x24;
  static constexpr uint8_t SET_LED_ALERT = 0x25; // KeySensorMCU::setLEDAlert unused 
  static constexpr uint8_t GET_SEVERAL_DATA = 0x26;

  static constexpr uint8_t RESET_EEPROM = 0x31;
  static constexpr uint8_t SET_DEBUG = 0x32; // 0:off, 1:on
  static constexpr uint8_t GET_VERSION = 0x33;

  static constexpr uint8_t GET_TPVAL = 0x41;
  static constexpr uint8_t GET_TPRAWVAL = 0x42;
  static constexpr uint8_t GET_TPMSG = 0x43;
  static constexpr uint8_t GET_TPPARAM = 0x44;
  static constexpr uint8_t SET_TPPARAM = 0x45;

  static constexpr uint8_t RESET = 0x81;

  // for controller internal use
  static constexpr uint8_t READY = 0xF1;
  static constexpr uint8_t MCURESET = 0xF2;
  static constexpr uint8_t SCAN = 0xF3;
  static constexpr uint8_t OLED = 0xF4;
  static constexpr uint8_t IS_ALIVE = 0xF5;
  static constexpr uint8_t TP_VAL = 0xF6; // unused yet
  static constexpr uint8_t TP_MSG = 0xF7; // same as GET_TPMSG
  static constexpr uint8_t I2C_DMSG = 0xF8;

  // parameters
  static constexpr uint8_t KEY_RELEASE = 1;
  static constexpr uint8_t KEY_PARTIALPRESS = 2;
  static constexpr uint8_t KEY_PRESS = 3;
  enum : uint8_t {
    LED_PTN_OFF = 0,
    LED_PTN_RED,
    LED_PTN_GREEN,
    LED_PTN_BLUE,
    LED_PTN_YELLOW,
    LED_PTN_CYAN,
    LED_PTN_MAGENTA,
    LED_PTN_WHITE,
    LED_PTN_RAINBOW,
    LED_PTN_MAX,
  };

  static constexpr uint8_t NKEYS = 7;
  static constexpr int TIMEOUT = 1000; // ms
};

/*
struct I2CKeyData { // for I2CCtrl::GET_VAL
  uint8_t val[I2CCtrl::NKEYS];
  uint8_t err;
};
*/

#pragma pack(push, 1)
struct I2CKeyState {
  uint8_t err;
  uint8_t pressed;
};

struct I2CKeyData {
  // send only key values 10 or greater
  // Key0 to Key6 order
  uint8_t val[I2CCtrl::NKEYS];
};

struct I2CLEDAttributes {
  uint8_t led_ptn_release;
  uint8_t led_ptn_partialpress;
  uint8_t led_ptn_press;
  uint8_t led_ptn_alert;
  uint8_t led_backlight; // use color of led_ptn_release, brightness is (led_backlight / 255)
  uint8_t led_brightness;
  bool backlight_enable; // not used in key sensor MCU. to include in the save data.
};

struct I2CLEDData {
  I2CLEDAttributes ledattr;
  uint8_t state[I2CCtrl::NKEYS];
};

struct I2CMCUData {
  I2CLEDData leddata;
  uint8_t keytest;
};

union I2CLoopData {
  uint16_t us[10];
  uint8_t b[20]; // RP2040 and ATTiny3224 are little-endian
};

struct I2CKeyTestData {
  uint8_t key;
  uint8_t val;
  uint8_t interval[2];
};

struct I2CMaxData { // GET_MSG, MET_TPMSG
  uint8_t data[30];
  uint8_t len;
};

struct I2CVersionInfo { // GET_VERSION
  uint8_t ver_major;
  uint8_t ver_minor;
  uint8_t ver_patch;
  uint8_t mcu_num;
  char str[24];
};
inline I2CVersionInfo I2C_vinfo;
inline auto setI2CVersionInfo = [](uint8_t major, uint8_t minor, uint8_t patch, uint8_t num) {
  I2C_vinfo.ver_major = major;
  I2C_vinfo.ver_minor = minor;
  I2C_vinfo.ver_patch = patch;
  I2C_vinfo.mcu_num = num;
  strncpy(I2C_vinfo.str, __DATE__ " " __TIME__, sizeof(I2C_vinfo.str) - 1);
  I2C_vinfo.str[sizeof(I2C_vinfo.str) - 1] = '\0';
  return((uint8_t *)&I2C_vinfo);
};

struct I2CTPData { // GET_TPVAL
  #define I2CTPData_TOUCH 0x01
  #define I2CTPData_SET_DELTAMAX(v) (((v) & 0x7f) << 1)
  #define I2CTPData_GET_DELTAMAX(v) (((v) >> 1) & 0x7f)
  uint8_t touch;
  uint16_t x, y;
};

struct I2CTPRawData { // GET_TPRAWVAL
  uint8_t x, y;
  uint8_t touch[5]; // rows array. low bit is col0
  int8_t v[5][5]; // [rows][cols]
};

struct I2CTPParam { // SET_TPPARAM
  uint8_t gain;
  uint8_t prescale;
  uint8_t oversamples;
  uint8_t rsel;
  uint16_t deadzone_x;
  uint16_t deadzone_y;
};
#pragma pack(pop)

static_assert(sizeof(I2CKeyState) == 2, "I2CKeyState size error");
static_assert(sizeof(I2CKeyData) == I2CCtrl::NKEYS, "I2CKeyData size error");
static_assert(sizeof(I2CLEDAttributes) == 7, "I2CLEDAttributes size error");
static_assert(sizeof(I2CLEDData) == 7 + I2CCtrl::NKEYS, "I2CLEDData size error");
static_assert(sizeof(I2CMCUData) == 7 + I2CCtrl::NKEYS + 1, "I2CMCUData size error");
static_assert(sizeof(I2CLoopData) == 20, "I2CLoopData size error");
static_assert(sizeof(I2CKeyTestData) == 4, "I2CKeyTestData size error");
static_assert(sizeof(I2CMaxData) == 31, "I2CMaxData size error");
static_assert(sizeof(I2CTPData) == 5, "I2CTPData size error");
static_assert(sizeof(I2CTPRawData) == 32, "I2CTPRawData size error");
static_assert(sizeof(I2CTPParam) == 8, "I2CTPParam size error");

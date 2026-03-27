#pragma once
#include <Adafruit_TinyUSB.h>
#include "custom_keymap.h"
#include "sysutils.h"
#include "functions.h"

enum : uint8_t {
  HID_RID_KEYBOARD = 1,
  HID_RID_MOUSE,
  HID_RID_WEBHID,
  HID_RID_TP_TOUCH,
  HID_RID_TP_MAXCOUNT,
  HID_RID_TP_DEVICEMODE,
  HID_RID_TP_SWITCH,
  HID_RID_TP_PTPHQA,
  HID_RID_TP_UMAPP,
  HID_RID_TP_MOUSE,
  HID_RID_MAX,
};

enum : uint8_t {
  REPORT_TYPE_BOOTKBD = 0,
  REPORT_TYPE_KBD,
  REPORT_TYPE_MOUSE,
  REPORT_TYPE_TOUCH,
  REPORT_TYPE_MAX,
};

#define HID_UNIT_CENTIMETER 0x10
#define HID_UNIT_SI_LINEAR  0x01
#define HID_UNIT_SECONDS    0x1001
#define HID_USAGE_BUTTON_1 0x01
#define HID_USAGE_BUTTON_2 0x02
#define HID_USAGE_BUTTON_3 0x03
#define HID_TP_PHYSICAL_X 1600
#define HID_TP_PHYSICAL_Y 800
#define HID_TP_LOGICAL_X 6400
#define HID_TP_LOGICAL_Y 3200
//#define HID_WEBHID_DATA_SIZE 259
#define HID_WEBHID_FRAGMENT_PAYLOAD_SIZE 60
// PAD physical 8+8cm x 8cm, logical 6400 x 3200, 40pt/mm

#define HID_REPORT_DESC_KEYBOARD_NKRO(rid) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                    ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD )                    ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                    ,\
    HID_REPORT_ID(rid)                                              \
    /* Modifier byte */ \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD )                     ,\
      HID_USAGE_MIN    ( 0xE0                                   )  ,\
      HID_USAGE_MAX    ( 0xE7                                   )  ,\
      HID_LOGICAL_MIN  ( 0                                      )  ,\
      HID_LOGICAL_MAX  ( 1                                      )  ,\
      HID_REPORT_COUNT ( 8                                      )  ,\
      HID_REPORT_SIZE  ( 1                                      )  ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )  ,\
    /* N-byte Keycodes */ \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD )                     ,\
      HID_USAGE_MIN    ( 0                                   )     ,\
      HID_USAGE_MAX    ( 0x53 /* 83 */                       )     ,\
      HID_LOGICAL_MIN  ( 0                                   )     ,\
      HID_LOGICAL_MAX  ( 1                                   )     ,\
      HID_REPORT_COUNT ( 84                                  )     ,\
      HID_REPORT_SIZE  ( 1                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )     ,\
      HID_REPORT_COUNT ( 4                                   )     ,\
      HID_INPUT        ( HID_CONSTANT                        ) ,\
      HID_USAGE_MIN    ( 0x72                                )     ,\
      HID_USAGE_MAX    ( 0x72                                )     ,\
      HID_LOGICAL_MIN  ( 0                                   )     ,\
      HID_LOGICAL_MAX  ( 1                                   )     ,\
      HID_REPORT_COUNT ( 1                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )     ,\
      HID_USAGE_MIN    ( 0x87                                )     ,\
      HID_USAGE_MAX    ( 0x8B                                )     ,\
      HID_LOGICAL_MIN  ( 0                                   )     ,\
      HID_LOGICAL_MAX  ( 1                                   )     ,\
      HID_REPORT_COUNT ( 5                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )     ,\
      HID_REPORT_COUNT ( 2                                   )     ,\
      HID_INPUT        ( HID_CONSTANT                            ) ,\
    /* Output 5-bit LED Indicator Kana | Compose | ScrollLock | CapsLock | NumLock */ \
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_LED                   )       ,\
      HID_USAGE_MIN    ( 1                                       ) ,\
      HID_USAGE_MAX    ( 5                                       ) ,\
      HID_REPORT_COUNT ( 5                                       ) ,\
      HID_REPORT_SIZE  ( 1                                       ) ,\
      HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ) ,\
      /* led padding */ \
      HID_REPORT_COUNT ( 1                                       ) ,\
      HID_REPORT_SIZE  ( 3                                       ) ,\
      HID_OUTPUT       ( HID_CONSTANT                            ) ,\
  HID_COLLECTION_END

#define HID_REPORT_DESC_MOUSE(rid) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP )                    ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE )                   ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
    HID_REPORT_ID   ( rid )           \
    HID_USAGE       ( HID_USAGE_DESKTOP_POINTER )               ,\
    HID_COLLECTION ( HID_COLLECTION_PHYSICAL )                  ,\
      HID_USAGE_PAGE ( HID_USAGE_PAGE_BUTTON )                   ,\
      HID_USAGE_MIN    ( 1                                   )     ,\
      HID_USAGE_MAX    ( 5                                   )     ,\
      HID_LOGICAL_MIN  ( 0                                   )     ,\
      HID_LOGICAL_MAX  ( 1                                   )     ,\
      HID_REPORT_COUNT ( 5                                   )     ,\
      HID_REPORT_SIZE  ( 1                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
      /* button padding */ \
      HID_REPORT_COUNT ( 1                                   )     ,\
      HID_REPORT_SIZE  ( 3                                   )     ,\
      HID_INPUT        ( HID_CONSTANT                        )     ,\
      /* X, Y */ \
      HID_USAGE_PAGE  (HID_USAGE_PAGE_DESKTOP)                 ,\
      HID_USAGE       (HID_USAGE_DESKTOP_X)                    ,\
      HID_USAGE       (HID_USAGE_DESKTOP_Y)                    ,\
      HID_LOGICAL_MIN ( 129 /* -127 */                 )   ,\
      HID_LOGICAL_MAX ( 127                              )   ,\
      HID_REPORT_SIZE  ( 8                                   )     ,\
      HID_REPORT_COUNT ( 2                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE |HID_RELATIVE) ,\
      /* Wheel */ \
      HID_USAGE       (HID_USAGE_DESKTOP_WHEEL)                ,\
      HID_LOGICAL_MIN ( 129 /* -127 */                      )   ,\
      HID_LOGICAL_MAX ( 127                                  )   ,\
      HID_REPORT_SIZE  ( 8                                   )     ,\
      HID_REPORT_COUNT ( 1                                   )     ,\
      HID_INPUT        ( HID_DATA | HID_VARIABLE |HID_RELATIVE) ,\
      /* Horizontal Wheel */ \
      HID_USAGE_N       (HID_USAGE_CONSUMER_AC_PAN, 2        )   ,\
      HID_LOGICAL_MIN  ( 129 /* -127 */                      )   ,\
      HID_LOGICAL_MAX  ( 127                                  )   ,\
      HID_REPORT_SIZE   ( 8                                   )     ,\
      HID_REPORT_COUNT  ( 1                                   )     ,\
      HID_INPUT         ( HID_DATA | HID_VARIABLE |HID_RELATIVE) ,\
    HID_COLLECTION_END                                        ,\
  HID_COLLECTION_END

#define HID_REPORT_TP_TOUCH() \
  HID_COLLECTION ( HID_COLLECTION_LOGICAL )    , \
  HID_LOGICAL_MIN ( 0 )                          , \
  HID_LOGICAL_MAX ( 1 )                          , \
  HID_USAGE       ( HID_USAGE_DIGITIZER_TOUCH_VALID ) , \
  HID_USAGE       ( HID_USAGE_DIGITIZER_TIP_SWITCH )   , \
  HID_REPORT_SIZE  ( 1 )                         , \
  HID_REPORT_COUNT ( 2 )                         , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_REPORT_COUNT ( 6 )                         , \
  HID_INPUT        ( HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_USAGE      ( HID_USAGE_DIGITIZER_CONTACT_IDENTIFIER )    , \
  HID_REPORT_SIZE  ( 8 )                         , \
  HID_REPORT_COUNT ( 1 )                         , \
  HID_LOGICAL_MAX_N( 255, 2 )                          , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP ) , \
  HID_USAGE       ( HID_USAGE_DESKTOP_X )         , \
  HID_UNIT_EXPONENT( 0xe /* -2 */)                      , \
  HID_UNIT         ( HID_UNIT_CENTIMETER | HID_UNIT_SI_LINEAR ) , \
  HID_LOGICAL_MAX_N( HID_TP_LOGICAL_X, 2)                      , \
  HID_PHYSICAL_MIN ( 0 )                          , \
  HID_PHYSICAL_MAX_N ( HID_TP_PHYSICAL_X, 2 )                        , \
  HID_REPORT_SIZE  ( 16 )                        , \
  HID_REPORT_COUNT ( 1 )                         , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_USAGE       ( HID_USAGE_DESKTOP_Y )         , \
  HID_LOGICAL_MAX_N( HID_TP_LOGICAL_Y, 2 )                      , \
  HID_PHYSICAL_MAX_N ( HID_TP_PHYSICAL_Y, 2 )                        , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_COLLECTION_END

#define HID_REPORT_TP_TOUCH_COMMON() \
  HID_UNIT_EXPONENT( 0xc /* -4 */ )                      , \
  HID_UNIT_N       ( HID_UNIT_SECONDS, 2 )            , \
  HID_PHYSICAL_MAX_N( 65535, 3 )                      , \
  HID_PHYSICAL_MIN  ( 0 )                         , \
  HID_LOGICAL_MAX_N( 65535, 3 )                      , \
  HID_LOGICAL_MIN  ( 0 )                         , \
  HID_REPORT_SIZE  ( 16 )                        , \
  HID_REPORT_COUNT ( 1 )                         , \
  HID_USAGE_PAGE  ( HID_USAGE_PAGE_DIGITIZER )     , \
  HID_USAGE       ( HID_USAGE_DIGITIZER_SCAN_TIME ) , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_USAGE       ( HID_USAGE_DIGITIZER_CONTACT_COUNT ) , \
  HID_LOGICAL_MAX  ( 3 )                        , \
  HID_REPORT_SIZE  ( 4 )                         , \
  HID_REPORT_COUNT ( 1 )                         , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON )        , \
  HID_USAGE      ( HID_USAGE_BUTTON_1 )       , \
  HID_USAGE      ( HID_USAGE_BUTTON_2 )       , \
  HID_USAGE      ( HID_USAGE_BUTTON_3 )       , \
  HID_LOGICAL_MAX  ( 1 )                          , \
  HID_REPORT_SIZE  ( 1 )                         , \
  HID_REPORT_COUNT ( 3 )                         , \
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
  HID_REPORT_COUNT ( 1 )                         , \
  HID_INPUT        ( HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE )

#define HID_REPORT_TP_MAXCOUNT(rid) \
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_DIGITIZER )     , \
    HID_REPORT_ID   ( rid )           \
    HID_USAGE       ( HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM ) , \
    HID_USAGE       ( HID_USAGE_DIGITIZER_PAD_TYPE ) , \
    HID_REPORT_SIZE  ( 4 )                         , \
    HID_REPORT_COUNT ( 2 )                         , \
    HID_LOGICAL_MIN  ( 0 )                         , \
    HID_LOGICAL_MAX  ( 15 )                         , \
    HID_FEATURE      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )

#define HID_REPORT_TP_PTPHQA(rid) \
    HID_USAGE_PAGE_N  ( HID_USAGE_PAGE_VENDOR, 2)          , \
    HID_REPORT_ID   ( rid )              \
    HID_USAGE       ( 0xC5 )                        , \
    HID_LOGICAL_MIN ( 0 )                          , \
    HID_LOGICAL_MAX_N ( 255, 2 )                        , \
    HID_REPORT_SIZE  ( 8 )                         , \
    HID_REPORT_COUNT_N ( 256, 2 )                       , \
    HID_FEATURE      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )

#define HID_REPORT_TP_DEVICEMODE(rid1, rid2) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DIGITIZER )                     , \
  HID_USAGE      ( HID_USAGE_DIGITIZER_DEVICE_CONFIGURATION )          , \
  HID_COLLECTION ( HID_COLLECTION_APPLICATION  )                  , \
    HID_REPORT_ID   ( rid1 )           \
    HID_USAGE     ( HID_USAGE_DIGITIZER_FINGER ) , \
    HID_COLLECTION( HID_COLLECTION_LOGICAL )      , \
      HID_USAGE       ( HID_USAGE_DIGITIZER_DEVICE_MODE ) , \
      HID_LOGICAL_MIN ( 0 )                          , \
      HID_LOGICAL_MAX ( 15 )                         , \
      HID_REPORT_SIZE  ( 8 )                         , \
      HID_REPORT_COUNT ( 1 )                         , \
      HID_FEATURE      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
    HID_COLLECTION_END                                          , \
    HID_COLLECTION( HID_COLLECTION_PHYSICAL )      , \
      HID_REPORT_ID   ( rid2 )      \
      HID_USAGE       ( HID_USAGE_DIGITIZER_SURFACE_SWITCH ) , \
      HID_USAGE       ( HID_USAGE_DIGITIZER_BUTTON_SWITCH ) , \
      HID_REPORT_SIZE  ( 1 )                         , \
      HID_REPORT_COUNT ( 2 )                         , \
      HID_LOGICAL_MAX  ( 1 )                          , \
      HID_FEATURE      ( HID_DATA |HID_VARIABLE | HID_ABSOLUTE ), \
      HID_REPORT_COUNT ( 6 )                         , \
      HID_FEATURE      ( HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE ), \
    HID_COLLECTION_END                                          , \
  HID_COLLECTION_END

#define HID_REPORT_DESC_TOUCHPAD() \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DIGITIZER )                     , \
  HID_USAGE      ( HID_USAGE_DIGITIZER_TOUCH_PAD )             , \
  HID_COLLECTION ( HID_COLLECTION_APPLICATION  )                  , \
    HID_REPORT_ID   ( HID_RID_TP_TOUCH )          \
    HID_USAGE       ( HID_USAGE_DIGITIZER_FINGER ) , \
    HID_REPORT_TP_TOUCH()                  , \
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_DIGITIZER )     , \
    HID_USAGE       ( HID_USAGE_DIGITIZER_FINGER ) , \
    HID_REPORT_TP_TOUCH()                  , \
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_DIGITIZER )     , \
    HID_USAGE       ( HID_USAGE_DIGITIZER_FINGER ) , \
    HID_REPORT_TP_TOUCH()                  , \
    HID_REPORT_TP_TOUCH_COMMON() , \
    HID_REPORT_TP_MAXCOUNT(HID_RID_TP_MAXCOUNT) , \
    HID_REPORT_TP_PTPHQA(HID_RID_TP_PTPHQA) , \
  HID_COLLECTION_END , \
  HID_REPORT_TP_DEVICEMODE(HID_RID_TP_DEVICEMODE, HID_RID_TP_SWITCH)

#define HID_REPORT_DESC_WEBHID(rid) \
  HID_USAGE_PAGE_N ( HID_USAGE_PAGE_VENDOR, 2 )                    ,\
  HID_USAGE      ( 0x01 )                                    ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )               ,\
    HID_REPORT_ID   ( rid )                                  \
    HID_USAGE       ( 2 )                                    ,\
    HID_LOGICAL_MIN ( 0 )                                    ,\
    HID_LOGICAL_MAX_N ( 255, 2 )                             ,\
    HID_REPORT_SIZE  ( 8 )                                   ,\
    HID_REPORT_COUNT_N ( HID_WEBHID_FRAGMENT_PAYLOAD_SIZE+2, 2 )                ,\
    HID_FEATURE      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
  HID_COLLECTION_END

/*
uint8_t PTPHQA_BLOB[] = {
0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, 0x0d, 0xbe, 0x57, 0x3c, 0xb6, 0x70, 0x09, 0x88,
0x07, 0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, 0x6c, 0xed, 0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2,
0x2e, 0x84, 0x1b, 0xe8, 0xb4, 0x51, 0x78, 0x43, 0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc,
0x47, 0x70, 0x1b, 0x59, 0x6f, 0x74, 0x43, 0xc4, 0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71,
0xc7, 0x95, 0x0e, 0x31, 0x55, 0x21, 0xd3, 0xb5, 0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8, 0x89, 0x19,
0x3e, 0xb3, 0xaf, 0x75, 0x81, 0x9d, 0x53, 0xb9, 0x41, 0x57, 0xf4, 0x6d, 0x39, 0x25, 0x29, 0x7c,
0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d, 0xa7, 0x26, 0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b,
0xbd, 0xff, 0x14, 0x67, 0xf2, 0x2b, 0xf0, 0x2a, 0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8,
0xc0, 0x8f, 0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, 0x0b, 0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51, 0xb7,
0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, 0xe8, 0x8a, 0x56, 0xf0, 0x8c, 0xaa, 0xfa, 0x35,
0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, 0x2b, 0x53, 0x5c, 0x69, 0x52, 0xd5, 0xc8, 0x73,
0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41, 0xe7, 0xff, 0x05, 0xd8, 0x2b, 0x79, 0x9a, 0xe2, 0x34, 0x60,
0x8f, 0xa3, 0x32, 0x1f, 0x09, 0x78, 0x62, 0xbc, 0x80, 0xe3, 0x0f, 0xbd, 0x65, 0x20, 0x08, 0x13,
0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, 0x5a, 0xc5, 0xd3, 0x7d, 0x98, 0xbe, 0x31, 0x48,
0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, 0xd6, 0xbf, 0xf2, 0xd3, 0x32, 0x2a, 0x9a, 0xe4,
0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33, 0x08, 0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2
};
*/

#define HID_NKRO_BYTE(code) ((code) >= 0x04 && (code) <= 0x53 ? (code) / 8 : ((code) == 0x72 || ((code) >= 0x87 && (code) <= 0x8B) ? 11 : 0))
#define HID_NKRO_BIT(code) ((code) >= 0x04 && (code) <= 0x53 ? (1 << ((code) % 8)) : ((code) == 0x72 ? (1 << 0) : ((code) >= 0x87 && (code) <= 0x8B ? (1 << ((code) - 0x86)) : 0)))
#define HID_NKRO_CODE(byte, bit) ([&]() { auto _tmp = (byte)*8+(bit); return(_tmp <= 0x53 ? _tmp : (_tmp == 0x58 ? 0x72 : (_tmp >= 0x59 ? _tmp + 0x2e : 0))); })()
#pragma pack(push, 1)
typedef struct {
  uint8_t modifier;
  uint8_t keycode[DEFS_HID_KEYBOARD_NKRO_BYTES];
} hid_report_keyboard_nkro_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t modifier;
  uint8_t reserved;
  uint8_t keycode[6];
} hid_report_keyboard_boot_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t button;
  int8_t x, y;
  int8_t wheel;
  int8_t hwheel;
} hid_report_mouse_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  #define TP_PADTYPE_CLICK (0x00<<4)
  #define TP_PADTYPE_PRESSURE (0x01<<4)
  #define TP_PADTYPE_DISCRETE (0x02<<4)
  #define TP_MAX_CONTACTS (3)
  #define TP_MAX_CONTACTS_SHIFT (0)
  #define TP_MAX_CONTACTS_MASK  (0xF)
  uint8_t max_count;
} hid_report_tp_maxcount_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t data[256];
} hid_report_tp_ptphqa_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t device_mode;
} hid_report_tp_feature_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  #define TP_STAT_TOUCH_VALID 0x01
  #define TP_STAT_TIP_SWITCH  0x02
  #define TP_CONTACT_ID_SHIFT (2)
  #define TP_CONTACT_ID_MASK  (0xF)
  uint8_t stat; // bit0: touch valid, bit1: tip switch
  uint8_t id; // contact id
  uint16_t x;
  uint16_t y;
} hid_report_tp_contact_t;
typedef struct {
  uint16_t scan_time;
  uint8_t contact_count_buttons;
  #define TP_CONTACT_COUNT_SHIFT (0)
  #define TP_CONTACT_COUNT_MASK  (0xF)
  #define TP_BUTTON1 (1<<4) // Pad Integrated Button
  #define TP_BUTTON2 (1<<5) // Left Button
  #define TP_BUTTON3 (1<<6) // Right Button
} hid_report_tp_base_t;
typedef struct {
  hid_report_tp_contact_t contacts[TP_MAX_CONTACTS];
  hid_report_tp_base_t base;
} hid_report_touchpad_t;
#pragma pack(pop)



typedef struct {
  size_t len;
  uint8_t *buf;
  size_t index;
  size_t stored_len;
  bool complete;
} hid_report_buffer_t;

enum webhid_fragment_type_t : uint8_t {
  WEBHID_FRAGMENT_END = 0,
  WEBHID_FRAGMENT_CONT,
};

#pragma pack(push, 1)
typedef struct {
  webhid_fragment_type_t type;
  uint8_t len;
  uint8_t payload[HID_WEBHID_FRAGMENT_PAYLOAD_SIZE];
} webhid_fragment_t;
#pragma pack(pop)


class HidManager {
public:
  typedef struct {
    Adafruit_USBD_HID hid;
    uint8_t instance_num;
    volatile bool report_complete;
    uint16_t not_ready_count;
  } hid_instance_t;

  HidManager() {}
  void begin();
  void remount() {
    if (TinyUSBDevice.mounted()) {
      TinyUSBDevice.detach();
      delay(10);
      TinyUSBDevice.attach();
    }
  }

  // keyboard
  bool keyboardSendReport();
  bool addKeycode(uint8_t code, uint8_t mod);
  bool isKeycodesChanged() { return(memcmp(&kbd_report, &kbd_report_prev, sizeof(kbd_report)) != 0); }
  void clearKeycode() {
    memset(&kbd_report, 0, sizeof(kbd_report));
    memset(&kbd_report_boot, 0, sizeof(kbd_report_boot));
    kbd_report_boot_index = 0;
  }
  void clearSendReport() {
    clearKeycode();
    keyboardSendReport();
  }
  static bool isKeyboardLEDChanged() { return (kbd_led_changed); }
  static void clearKeyboardLEDChanged() { kbd_led_changed = false; }
  static bool isNumlock() { return (kbd_led_bm & KEYBOARD_LED_NUMLOCK); }
  static bool isCapslock() { return (kbd_led_bm & KEYBOARD_LED_CAPSLOCK); }
  static bool isScrolllock() { return (kbd_led_bm & KEYBOARD_LED_SCROLLLOCK); }

  bool hidSendReport(hid_instance_t &instance, uint8_t report_id, void const *report, uint8_t len);

  // mouse
  bool mouseSendReport(uint8_t button, int8_t x, int8_t y, int8_t wheel, int8_t hwheel);

  // touchpad
  bool tpmouseSendReport(TPPoint &left, TPPoint &right);
  bool touchpadSendReport(bool firsttouch, TPPoint &left, TPPoint &right);
  bool tpmouseScroll(int8_t distance);

  // webhid
  static void webhidSetBuffer(uint8_t *reqbuf, size_t reqbuflen, uint8_t *resbuf, size_t resbuflen) {
    webhid_req_bufinfo.len = reqbuflen;
    webhid_req_bufinfo.buf = reqbuf;
    webhid_req_bufinfo.index = 0;
    webhid_req_bufinfo.complete = false;

    webhid_res_bufinfo.len = resbuflen;
    webhid_res_bufinfo.buf = resbuf;
    webhid_res_bufinfo.index = 0;
    webhid_res_bufinfo.complete = false;
  }
  bool webhidReceiveRequest(uint8_t *buf, size_t len);
  bool webhidSendResponse(uint8_t *buf, size_t len);

  static bool getKeyboardProtocolFeature() { return kbd_protocol_feature; }

  friend void tud_hid_set_report_cb(uint8_t instance, hid_report_type_t report_type, uint8_t const* report, uint16_t len);
  friend void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol);
  friend bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate);
  friend void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len);
  friend void tud_hid_report_failed_cb(uint8_t instance, hid_report_type_t report_type, uint8_t const* report, uint16_t xferred_bytes);
  friend void F_Key::setKeyboardProtocolFeature(bool);
  friend bool F_Key::toggleKeyboardProtocolFeature();

private:
  inline static const uint8_t report_desc_bootkbd[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(),
  };
  static void setReportCallbackBootkbd(uint8_t, hid_report_type_t, uint8_t const*, uint16_t);
  static uint16_t getReportCallbackBootkbd(uint8_t, hid_report_type_t, uint8_t*, uint16_t);

  inline static const uint8_t report_desc_composite[] = {
    HID_REPORT_DESC_KEYBOARD_NKRO(HID_RID_KEYBOARD),
    HID_REPORT_DESC_MOUSE(HID_RID_MOUSE),
    HID_REPORT_DESC_WEBHID(HID_RID_WEBHID),
    HID_REPORT_DESC_TOUCHPAD(),
  };
  static void setReportCallbackComposite(uint8_t, hid_report_type_t, uint8_t const*, uint16_t);
  static uint16_t getReportCallbackComposite(uint8_t, hid_report_type_t, uint8_t*, uint16_t);

  // common

  // keyboard
  inline static volatile bool kbd_protocol_feature = true;
  inline static volatile uint8_t kbd_led_bm;
  inline static volatile bool kbd_led_changed;
  hid_report_keyboard_nkro_t kbd_report = {};
  hid_report_keyboard_nkro_t kbd_report_prev = {};
  hid_report_keyboard_boot_t kbd_report_boot = {};
  uint8_t kbd_report_boot_index = 0;

  // touchpad
  unsigned long last_proc_100us = 0;
  unsigned long scantime_reference = 0;
  void setTouchParam(hid_report_touchpad_t &tp_report, bool tipsw, uint8_t index, uint8_t id, uint16_t x, uint16_t y) {
    tp_report.contacts[index].stat = (tipsw ? TP_STAT_TIP_SWITCH | TP_STAT_TOUCH_VALID : TP_STAT_TOUCH_VALID);
    tp_report.contacts[index].id = id;
    tp_report.contacts[index].x = x;
    tp_report.contacts[index].y = y;
  }
  void printTouchpadReport(hid_report_touchpad_t &tp_report) {
    uint8_t contact_count = tp_report.base.contact_count_buttons & 3;
    LOG_DEBUG(
      "contact Scan Time:", tp_report.base.scan_time,
      ", Count:", contact_count,
      ", Btn1:", (tp_report.base.contact_count_buttons & TP_BUTTON1 ? "ON" : "OFF"),
      ", Btn2:", (tp_report.base.contact_count_buttons & TP_BUTTON2 ? "ON" : "OFF"),
      ", Btn3:", (tp_report.base.contact_count_buttons & TP_BUTTON3 ? "ON" : "OFF"));
    for(uint8_t i = 0; i < TP_MAX_CONTACTS; i++) {
      LOG_DEBUG(
        "  ", i,
        ": stat=", tp_report.contacts[i].stat & 3,
        ", id=", tp_report.contacts[i].id,
        ", x=", tp_report.contacts[i].x,
        ", y=", tp_report.contacts[i].y);
    }
  }

  // for mouse mode
  uint16_t mm_x_l = 0, mm_y_l = 0, mm_x_r = 0, mm_y_r = 0;

  // webhid
  inline static volatile hid_report_buffer_t webhid_req_bufinfo = {0, nullptr, 0, false};
  inline static volatile hid_report_buffer_t webhid_res_bufinfo = {0, nullptr, 0, false};

  // manager
  hid_instance_t boot_keyboard = { Adafruit_USBD_HID() , 0, true, 0 };
  hid_instance_t composite = { Adafruit_USBD_HID() , 0, true, 0 };

  /*
    Often HID Report causes stacking TinyUSB.
    Implement multiple measures to avoid blocking main loop by TinyUSB stack.
    1.Check completion of report submission with tud_hid_report_complete_cb().
    2.Check hid.ready().
    3.Set Adafruit_USBD_HID interrupt IN interval to 2 ms or more.
    //4.Call TinyUSBDevice.task() before F_* action.
    5.Control Touchpad report interval to 5 ms or more in F_Touchpad.
  */
  bool waitHidReady(hid_instance_t &instance);
};

typedef struct {
  HidManager hidmanager;
  CircularBuffer debug_hidcallback, error_hidcallback;
} appcontext_hid_t;
appcontext_hid_t &appctxHid();
inline appcontext_hid_t ctxhid = appctxHid();

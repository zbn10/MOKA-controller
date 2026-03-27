#pragma once
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "i2c_control.h"
#include "sysutils.h"
#include "custom_keymap.h"
#include "hid.h"

#pragma pack(push, 1)
typedef struct {
  uint8_t profile;
  uint8_t oslayout;
  uint8_t map[KEYMAPTABLE_MAX][DEFS_NMAGKEYS];
  struct {
    uint8_t key;
    uint8_t state;
    uint8_t layer;
    uint8_t func;
  } funit[10];
  //uint8_t tapadv_enable;
} webhid_keymap_t;

/*
typedef struct {
  uint8_t profile;
  uint8_t thresh_partial_press[DEFS_NMAGKEYS];
  uint8_t thresh_press[DEFS_NMAGKEYS];
  uint8_t thresh_partial2release[DEFS_NMAGKEYS];
  uint8_t thresh_press2release[DEFS_NMAGKEYS];
  uint16_t duration_hold[DEFS_NMAGKEYS];
  uint8_t control[DEFS_NMAGKEYS];
} webhid_magattr_t;
 */
typedef struct {
  uint8_t profile;
  uint8_t thresh_partial_press;
  uint8_t thresh_press;
  uint8_t thresh_partial2release;
  uint8_t thresh_press2release;
  uint8_t duration_hold_high;
  uint8_t duration_hold_low;
  //uint8_t control;
} webhid_magattr_t;

typedef struct {
  uint8_t profile; // used for Default, Saved or Active profile selection
  uint8_t primary;
  uint8_t led_ptn_release;
  uint8_t led_ptn_partialpress;
  uint8_t led_ptn_press;
  uint8_t led_ptn_alert;
  uint8_t led_backlight;
  uint8_t led_brightness;
  //bool backlight_enable;
  //uint32_t color[LEDTYPE_MAX];
  uint8_t color[LEDTYPE_MAX][3]; // R G B
} webhid_common_t;

enum : uint8_t {
  WEBHID_CMD_NONE = 0,
  WEBHID_CMD_RESPONSE,
  WEBHID_CMD_FETCH = 11,
  WEBHID_CMD_APPLY = 21,
  WEBHID_CMD_SAVE = 31,
};

enum : uint8_t {
  WEBHID_RES_ACK = 0,
  WEBHID_RES_NACK,
  WEBHID_RES_CHG, // accept and changed
  WEBHID_RES_INVALID_CMD,
  WEBHID_RES_INVALID_PRIMARY,
  WEBHID_RES_INVALID_PROFILE,
  WEBHID_RES_INVALID_OSLAYOUT,
  WEBHID_RES_INVALID_KEYMAP,
  WEBHID_RES_INVALID_INDKEY,
  WEBHID_RES_INVALID_MAGATTR,
  WEBHID_RES_INVALID_KEYLED,
  WEBHID_RES_INVALID_LED,
  WEBHID_RES_INVALID_SAVE,
  WEBHID_RES_ERROR = 127,
};


typedef struct { // used for fetch request, save request and apply response
  uint8_t cmd;
  uint8_t primaryProfile; // used for save request.
  uint8_t profile; // used for save request and fetch request.
  uint8_t osLayout; // used for fetch request.
} webhid_info_t;

typedef struct { // used for apply request, fetch response.
  webhid_info_t info;
  // primaryProfile; // used for fetch request.
  // profile; // used for nothing currently.
  // osLayout;
  uint8_t keymap[KEYMAPTABLE_MAX][DEFS_NMAGKEYS];
  struct {
    uint8_t key;
    uint8_t state;
    uint8_t layer;
    uint8_t func;
  } funit[10];
  uint8_t magattr_pp;
  uint8_t magattr_pp2r;
  uint8_t magattr_p;
  uint8_t magattr_p2r;
  uint8_t magattr_hold_high;
  uint8_t magattr_hold_low;
  uint8_t keyled_release;
  uint8_t keyled_partialpress;
  uint8_t keyled_press;
  uint8_t keyled_error;
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  } led[7];
  uint8_t brightness;
  uint8_t backlight_enable;
} webhid_data_t;
#pragma pack(pop)



class WebHID {
public:
  void begin() {
    HidManager::webhidSetBuffer(webhid_reqbuf, sizeof(webhid_reqbuf), webhid_resbuf, sizeof(webhid_resbuf));
  }
  bool fetch(webhid_info_t &);
  bool apply(webhid_data_t &);
  bool save(webhid_info_t &);
  void progress(webhid_data_t &);

private:
  uint8_t webhid_reqbuf[sizeof(webhid_data_t)];
  uint8_t webhid_resbuf[sizeof(webhid_data_t)];
};

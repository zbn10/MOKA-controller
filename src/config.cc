#include <EEPROM.h>
#include "debuglog.h"
#include <DebugLog.h>
#include <Adafruit_NeoPixel.h>
#include "controller.h"
#include "functions.h"
#include "config.h"
#include "custom_keymap.h"
#include "sysutils.h"

Keymap Config::default_keymap[OSLAYOUT_MAX] = {
{
  OSLAYOUT_US,
  //(KeymapOSLayoutType)3, // for validation check
  {
    { // layer 0
      F_TAB, F_q, F_w, F_e, F_r, F_t,               F_y, F_u, F_i, F_o, F_p, F_BS,
      F_CTRL_LEFT, F_a, F_s, F_d, F_f, F_g,         F_h, F_j, F_k, F_l, F_SEMICOLON, F_QUOTE,
#if defined(SUPPORT_STAGGERED)
      F_SHIFT_LEFT, F_z, F_x, F_c, F_v, F_SPACE,    F_b, F_n, F_m, F_COMMA, F_PERIOD, F_SLASH,
#else
      F_SHIFT_LEFT, F_NONE, F_z, F_x, F_c, F_v,     F_n, F_m, F_COMMA, F_PERIOD, F_SLASH, F_NONE,
#endif
      F_LAYER_1, F_LAYER_2, F_LAYER_3,              F_LAYER_3, F_LAYER_2, F_LAYER_1,
    },
    { // layer 1, mainly symbols of layer0, similar to 'shift layer'
      F_ESC, F_Q, F_W, F_E, F_R, F_T,               F_Y, F_U, F_I, F_O, F_P, F_DELETE,
      F_CTRL_LEFT, F_A, F_S, F_D, F_F, F_G,         F_H, F_J, F_K, F_L, F_COLON, F_DQUOTE,
#if defined(SUPPORT_STAGGERED)
      F_SHIFT_LEFT, F_Z, F_X, F_C, F_V, F_SPACE,    F_B, F_N, F_M, F_ANGLE_LEFT, F_ANGLE_RIGHT, F_QUESTION,
      F_GUI_LEFT, F_ALT_LEFT, F_LAYER_HOLD,                 F_LAYER_3, F_LAYER_2, F_LAYER_1,
#else
      F_SHIFT_LEFT, F_NONE, F_Z, F_X, F_C, F_V,     F_N, F_M, F_ANGLE_LEFT, F_ANGLE_RIGHT, F_QUESTION, F_NONE,
      F_SPACE, F_SPACE, F_NONE,                     F_B, F_ENTER, F_ENTER,
#endif
    },
    { // layer 2, other major symbols
      F_PRINT_SCREEN,  F_EXCL, F_NONE,     F_ARROW_UP,   F_NONE,        F_PAREN_LEFT,       F_MINUS, F_EQUAL, F_GRAVE, F_BRACKET_LEFT, F_BRACKET_RIGHT, F_BACKSLASH,
      F_CTRL_LEFT, F_AT, F_ARROW_LEFT, F_ARROW_DOWN, F_ARROW_RIGHT, F_PAREN_RIGHT,          F_UNDERSCORE, F_PLUS, F_TILDE, F_BRACE_LEFT, F_BRACE_RIGHT, F_PIPE,
      F_SHIFT_LEFT, F_NONE, F_NONE, F_HASH, F_DOLLAR, F_PERCENT,                            F_CARET, F_AMPERSAND, F_ASTERISK, F_COPILOT, F_PAGE_UP, F_PAGE_DOWN,
      F_TOUCHPAD_BTN_LEFT, F_TOUCHPAD_BTN_RIGHT, F_LAYER_HOLD,                                                 F_LAYER_3, F_LAYER_2, F_LAYER_1,
    },
    { // layer 3, other major symbols
      F_TGL_OSLAYOUT, F_CHG_PROFD, F_CHG_PROF0, F_CHG_PROF1, F_CHG_PROF2, F_CHG_PROF3,      F_MACRO00, F_MACRO01, F_MACRO02, F_MACRO03, F_MACRO04, F_BS,
      F_CTRL_LEFT, F_MBTN_BW, F_MBTN_LEFT, F_M_WHEEL, F_MBTN_RIGHT, F_MBTN_FW,              F_PLUS, F_MINUS, F_ASTERISK, F_SLASH, F_NONE, F_KBD_RESET,
      F_SHIFT_LEFT, F_1, F_2, F_3, F_4, F_5,                                                F_6, F_7, F_8, F_9, F_0, F_SLASH,
      F_TOUCHPAD_ENABLE, F_KLED_BACKLIGHT, F_LAYER_HOLD,                                               F_LAYER_3, F_LAYER_2, F_LAYER_1,
    },
  },
  { // custom keys
#if !defined(SUPPORT_STAGGERED)
  { 39, KS_TAP, LAYER_0, F_b},
#endif
  { 36, KS_TAP, LAYER_0, F_SPACE },
  { 41, KS_TAP, LAYER_0, F_ENTER },
  { 0, KS_PARTIAL, LAYER_0, F_F2 },
  { 12, KS_PARTIAL, LAYER_0, F_F7 },
  #if defined(SUPPORT_PHOTOIC_INFRARED)
  { CK_PHOTOIC, KS_VALUE, LAYER_0, F_BRIGHTNESS },
  { CK_INFRAREDSENSOR, KS_VALUE, LAYER_0, F_JIGGLER },
#endif
#if defined(SUPPORT_TOUCHPAD)
  { CK_TOUCHPAD_LEFT, KS_VALUE, LAYER_1, F_TOUCHPAD_LEFT2X },
  { CK_TOUCHPAD_RIGHT, KS_VALUE, LAYER_1, F_TOUCHPAD_RIGHT2X },
#endif
  },
  1, // tapadv_enable
},
{
  OSLAYOUT_JP,
  {
    { // layer 0
      F_TAB, F_q, F_w, F_e, F_r, F_t,               F_y, F_u, F_i, F_o, F_p, F_BS,
      F_CTRL_LEFT, F_a, F_s, F_d, F_f, F_g,         F_h, F_j, F_k, F_l, F_SEMICOLON, F_COLON,
#if defined(SUPPORT_STAGGERED)
      F_SHIFT_LEFT, F_z, F_x, F_c, F_v, F_SPACE,    F_b, F_n, F_m, F_COMMA, F_PERIOD, F_SLASH,
#else
      F_SHIFT_LEFT, F_NONE, F_z, F_x, F_c, F_v,     F_n, F_m, F_COMMA, F_PERIOD, F_SLASH, F_NONE,
#endif
      F_LAYER_1, F_LAYER_2, F_LAYER_3,              F_LAYER_3, F_LAYER_2, F_LAYER_1,
    },
    { // layer 1, mainly symbols of layer0, similar to 'shift layer'
      F_ESC, F_Q, F_W, F_E, F_R, F_T,               F_Y, F_U, F_I, F_O, F_P, F_DELETE,
      F_CTRL_LEFT, F_A, F_S, F_D, F_F, F_G,         F_H, F_J, F_K, F_L, F_PLUS, F_ASTERISK,
#if defined(SUPPORT_STAGGERED)
      F_SHIFT_LEFT, F_Z, F_X, F_C, F_V, F_SPACE,    F_B, F_N, F_M, F_ANGLE_LEFT, F_ANGLE_RIGHT, F_QUESTION,
      F_GUI_LEFT, F_ALT_LEFT, F_LAYER_HOLD,                 F_LAYER_3, F_LAYER_2, F_LAYER_1,
#else
      F_SHIFT_LEFT, F_NONE, F_Z, F_X, F_C, F_V,     F_N, F_M, F_ANGLE_LEFT, F_ANGLE_RIGHT, F_QUESTION, F_NONE,
      F_SPACE, F_SPACE, F_NONE,                     F_B, F_ENTER, F_ENTER,
#endif
    },
    { // layer 2, other major symbols
      F_HANZEN, F_PRINT_SCREEN,  F_NONE,     F_ARROW_UP,   F_NONE,        F_COPILOT,       F_MINUS, F_CARET, F_BACKSLASH, F_AT, F_BRACKET_LEFT, F_BRACKET_RIGHT,
      F_CTRL_LEFT, F_NONE, F_ARROW_LEFT, F_ARROW_DOWN, F_ARROW_RIGHT, F_UNDERSCORE,         F_EQUAL, F_TILDE, F_PIPE, F_GRAVE, F_BRACE_LEFT, F_BRACE_RIGHT,
      F_SHIFT_LEFT, F_EXCL, F_DQUOTE, F_HASH, F_DOLLAR, F_PERCENT,                  F_AMPERSAND, F_QUOTE, F_PAREN_LEFT, F_PAREN_RIGHT, F_PAGE_UP, F_PAGE_DOWN,
      F_TOUCHPAD_BTN_LEFT, F_TOUCHPAD_BTN_RIGHT, F_LAYER_HOLD,                                         F_LAYER_3, F_LAYER_2, F_LAYER_1,
    },
    { // layer 3, other major symbols
      F_TGL_OSLAYOUT, F_CHG_PROFD, F_CHG_PROF0, F_CHG_PROF1, F_CHG_PROF2, F_CHG_PROF3,      F_MACRO00, F_MACRO01, F_MACRO02, F_MACRO03, F_MACRO04, F_BS,
      F_CTRL_LEFT, F_MBTN_BW, F_MBTN_LEFT, F_M_WHEEL, F_MBTN_RIGHT, F_MBTN_FW,              F_PLUS, F_MINUS, F_ASTERISK, F_SLASH, F_NONE, F_KBD_RESET,
      F_SHIFT_LEFT, F_1, F_2, F_3, F_4, F_5,                                                F_6, F_7, F_8, F_9, F_0, F_SLASH,
      F_TOUCHPAD_ENABLE, F_KLED_BACKLIGHT, F_LAYER_HOLD,                                               F_LAYER_3, F_LAYER_2, F_LAYER_1,
    },
  },
  { // custom keys
#if !defined(SUPPORT_STAGGERED)
  { 39, KS_TAP, LAYER_0, F_b},
#endif
  { 36, KS_TAP, LAYER_0, F_SPACE },
  { 41, KS_TAP, LAYER_0, F_ENTER },
  { 0, KS_PARTIAL, LAYER_0, F_F2 },
  { 12, KS_PARTIAL, LAYER_0, F_F7 },
#if defined(SUPPORT_PHOTOIC_INFRARED)
  { CK_PHOTOIC, KS_VALUE, LAYER_0, F_BRIGHTNESS },
  { CK_INFRAREDSENSOR, KS_VALUE, LAYER_0, F_JIGGLER },
#endif
#if defined(SUPPORT_TOUCHPAD)
  { CK_TOUCHPAD_LEFT, KS_VALUE, LAYER_1, F_TOUCHPAD_LEFT2X },
  { CK_TOUCHPAD_RIGHT, KS_VALUE, LAYER_1, F_TOUCHPAD_RIGHT2X },
#endif
  },
  1, // tapadv_enable
},
};

// individual common units
const FunctionUnit CustomKeymap::funit_common[] = {
  // don't change below.
  // because they are used in the emergency mode.
  { CK_TACTSW, KS_HOLDLL, LAYER_0, F_LAYER_CALIBRATION },
  { CK_TACTSW, KS_PRESS, LAYER_EMERGENCY, F_EXIT_EMERGENCY },
  { CK_TACTSW, KS_PRESS, LAYER_CALIBRATION, F_LAYER_CALIBRATION },

  // common functons
  { CK_TACTSW, KS_PRESS, LAYER_0, F_CHG_OLED },
#if defined(SUPPORT_TINYMOUSE)
  { CK_JOYSTICKMOUSE, KS_VALUE, LAYER_3, F_MOUSE_MOVE },
#endif
#if defined(SUPPORT_TOUCHPAD)
  { CK_TOUCHPAD_LEFT, KS_VALUE, LAYER_0, F_TOUCHPAD_LEFT },
  { CK_TOUCHPAD_RIGHT, KS_VALUE, LAYER_0, F_TOUCHPAD_RIGHT },
#endif
};
const uint8_t CustomKeymap::num_funit_common = sizeof(funit_common) / sizeof(funit_common[0]);


KeyswAttributes Config::default_magkeyattr[DEFS_NMAGKEYS] = {
  // row 0
  // for validation check below
  //{KeyswConst::ThrPress, KeyswConst::ThrPartialPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},

  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  // row 1
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},

  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  // row 2
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},

  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  // row 3
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},

  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
  {KeyswConst::ThrPartialPress, KeyswConst::ThrPress, KeyswConst::ThrPartial2Release, KeyswConst::ThrPress2Release, KeyswConst::DurHold, KC_NONE},
};

/*
 * 5th parameter LED brightness is used for both key LED and system LED.
 */
I2CLEDAttributes Config::default_ledattr = {  
  //10, // for validation check
  LEDConst::LED_PTN_RELEASE,
  LEDConst::LED_PTN_PARTIALPRESS,
  LEDConst::LED_PTN_PRESS,
  LEDConst::LED_PTN_ALERT,
  LEDConst::LED_BACKLIGHT,
  LEDConst::LED_BRIGHTNESS,
  0, // backlight_enable
};

ConfigKeyboardLED Config::default_keyboardled = {
  LEDConst::LED_COLOR_CALIBRATION,
  LEDConst::LED_COLOR_SYSTEM,
  LEDConst::LED_COLOR_MCUERR,
  LEDConst::LED_COLOR_KEYERR,
  LEDConst::LED_COLOR_NUMLOCK,
  LEDConst::LED_COLOR_CAPSLOCK,
  LEDConst::LED_COLOR_SCROLLLOCK,
};

MacroSequence Config::default_macros[DEFS_NMACROS] = {
  // Macro 0
  {2,
    {
      {50, 0, 10, F_T, F_y, F_p, F_e, F_SPACE, F_Y, F_o, F_u, F_r, F_SPACE},
      {50, 0, 9, F_P, F_a, F_s, F_s, F_W, F_o, F_r, F_d, F_ENTER},
    },
  },
  // Macro 1
  {3,
    {
      {20, 1, 2, F_GUI_LEFT, F_r},
      {200, 0, 0},
      {20, 0, 9, F_c, F_a, F_l, F_c, F_PERIOD, F_e, F_x, F_e, F_ENTER},
    },
  },
  // Macro 2
  {1,
    {
      {100, 1, 2, F_GUI_LEFT, F_TAB},
    },
  },
  // Macro 3
  {0},
  // Macro 4
  {0},
  // Macro 5
  {0},
};

const oef_param_t &Config::default_tpoef = F_Touchpad::oef_default;

KeyswAttributes (&Config::active_magkeyattr)[DEFS_NMAGKEYS] = ctxmagsw.magsw_attr_list;
oef_param_t &Config::active_tpoef_left = ctxreporter.f_touchpad_reporter.getOEFParamRef(TP_LEFT);
oef_param_t &Config::active_tpoef_right = ctxreporter.f_touchpad_reporter.getOEFParamRef(TP_RIGHT);

/* save active tables to the EEPROM
  * prof: profile number or PROFILE_PRIMARY
*/
bool Config::saveConfiguration(uint8_t prof) {
  if(prof == PROFILE_PRIMARY) {
    prof = primary_profile;
  } else if(prof >= PROFILE_MAX) {
    LOG_ERROR("Invalid profile number: ", prof);
    return false;
  }

  // dont change primary_profile. its independent.
  loaded_profile = prof;

  EEPROM.begin(EEPROM_SIZE);

  EEPROM.put(0, primary_profile);

  size_t offset = sizeof(primary_profile) + prof * (sizeof(vtag) + sizeof(active_keymap) + sizeof(active_magkeyattr));
  EEPROM.put(offset, vtag); // Configuration version is saved at the beginning of each profile.
  offset += sizeof(vtag);
  EEPROM.put(offset, active_keymap);
  offset += sizeof(active_keymap);
  EEPROM.put(offset, active_magkeyattr);

  offset = sizeof(primary_profile) + PROFILE_MAX * (sizeof(vtag) + sizeof(active_keymap) + sizeof(active_magkeyattr));
  EEPROM.put(offset, active_ledattr);
  offset += sizeof(active_ledattr);

  EEPROM.put(offset, active_keyboardled);
  offset += sizeof(active_keyboardled);

  EEPROM.put(offset, active_macros);
  offset += sizeof(active_macros);

  EEPROM.put(offset, active_tpoef_left);
  offset += sizeof(active_tpoef_left);
  EEPROM.put(offset, active_tpoef_right);
  offset += sizeof(active_tpoef_right);

  EEPROM.end();

#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("saved to profile ", prof, ", EEPROM size: ", offset);
  }
#endif
  return true;
}

bool Config::savePrimaryProfileNum(uint8_t prof) {
  if(prof >= PROFILE_MAX) {
    LOG_ERROR("Invalid primary profile number: ", prof);
    return false;
  }
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, prof);
  EEPROM.end();
  primary_profile = prof;
  return true;
}

bool Config::saveConfigLED() {
  EEPROM.begin(EEPROM_SIZE);

  size_t offset = sizeof(primary_profile) + PROFILE_MAX * (sizeof(vtag) + sizeof(active_keymap) + sizeof(active_magkeyattr));
  EEPROM.put(offset, active_ledattr);
  offset += sizeof(active_ledattr);

  EEPROM.put(offset, active_keyboardled);
  offset += sizeof(active_keyboardled);

  EEPROM.end();
  return true;
}

bool Config::saveTPOEFParam() {
  EEPROM.begin(EEPROM_SIZE);

  size_t offset = sizeof(primary_profile) + PROFILE_MAX * (sizeof(vtag) + sizeof(active_keymap) + sizeof(active_magkeyattr))
    + sizeof(active_ledattr) + sizeof(active_keyboardled) + sizeof(active_macros);
  EEPROM.put(offset, active_tpoef_left);
  offset += sizeof(active_tpoef_left);
  EEPROM.put(offset, active_tpoef_right);
  offset += sizeof(active_tpoef_right);

  EEPROM.end();
  return true;
}

bool Config::loadConfiguration(uint8_t origin, uint8_t type) { // load to the active tables
  if(origin == PROFILE_ACTIVE) return false;

  bool r;
  switch(type) {
    case KM_ALL:
      r = loadConfiguration(
        origin, PROFILE_ACTIVE,
        active_keymap, active_magkeyattr, active_ledattr, active_keyboardled, active_macros, active_tpoef_left, active_tpoef_right);
        ctxsys.f_systemled.applyLEDSettings();
        break;
    case KM_KEYMAP:
      r = loadConfiguration(
        origin, PROFILE_ACTIVE,
        active_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right);
        break;
    case KM_MAGATTR:
      r = loadConfiguration(
        origin, PROFILE_ACTIVE,
        tmp_keymap, active_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right);
        break;
    case KM_COMMON:
      r = loadConfiguration(
        origin, PROFILE_ACTIVE,
        tmp_keymap, tmp_magkeyattr, active_ledattr, active_keyboardled, active_macros, active_tpoef_left, active_tpoef_right);
        ctxsys.f_systemled.applyLEDSettings();
        break;
    default:
      return false;
  }

  return(r);
}

/* load key configuration from origin to the specified tables
  * origin: PROFILE_ACTIVE, PROFILE_DEFAULT, PROFILE_PRIMARY, 0-3

  keymap validation and optimize.
  EEPROM/Default -> Active : validate and optimize
  EEPROM/Default -> other : e.g. output to webhid, not required
  Active -> EEPROM : not required because optimized already
  webhid -> Active : validate and optimize in webhid process
  */

bool Config::loadConfiguration(
  uint8_t origin, uint8_t dest,
  Keymap &km, KeyswAttributes (&keyattr)[DEFS_NMAGKEYS],
  I2CLEDAttributes &ledattr, ConfigKeyboardLED &keyboardled, MacroSequence (&macros)[DEFS_NMACROS],
  oef_param_t &tpoef_left, oef_param_t &tpoef_right)
{
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, primary_profile); // load always
  EEPROM.end();
  if(primary_profile >= PROFILE_MAX) {
    LOG_ERROR("Invalid primary profile number: ", primary_profile, ", resetting to 0");
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, (uint8_t)0);
    EEPROM.end();
    primary_profile = 0;
  }

  uint8_t prof;
  if(origin == PROFILE_ACTIVE) {
    km = active_keymap;
    //memcpy(keyattr, active_keyattr, sizeof(active_keyattr));
    memcpy(keyattr, active_magkeyattr, sizeof(keyattr));
    ledattr = active_ledattr;
    keyboardled = active_keyboardled;
    memcpy(macros, active_macros, sizeof(macros));
    tpoef_left = active_tpoef_left;
    tpoef_right = active_tpoef_right;
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Loaded Keymap", "Active");
    return true;
  }

  else if(origin == PROFILE_DEFAULT) {
    km = default_keymap[ctxkeymap.keymap.getOSLayout()];

    CustomKeymap::validateKeymap(km);

    memcpy(keyattr, default_magkeyattr, sizeof(keyattr));
    MagswStateMachine::validateKeyattr(keyattr);

    ledattr = default_ledattr;
    F_SystemLED::validateLEDAttr(ledattr);

    keyboardled = default_keyboardled;

    memcpy(macros, default_macros, sizeof(macros));
    for(uint8_t i = 0; i < DEFS_NMACROS; i++) {
      F_KeyMacro::validate(macros[i]);
    }

    tpoef_left = default_tpoef;
    tpoef_right = default_tpoef;

    if(dest == PROFILE_ACTIVE) {
      loaded_profile = PROFILE_DEFAULT;
      ctxkeymap.keymap.optimizeKeymap();
    }
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Loaded Keymap", "Default");
    return true;
  }

  else if(origin == PROFILE_PRIMARY) {
    prof = primary_profile;
  } else if(origin < PROFILE_MAX) {
    prof = origin;
  } else {
    LOG_ERROR("Invalid origin ", origin);
    return false;
  }

#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("sizeof(keymap): ", sizeof(km));
    LOG_DEBUG("sizeof(keyattr): ", sizeof(keyattr));
    LOG_DEBUG("sizeof(ledattr): ", sizeof(ledattr));
    LOG_DEBUG("sizeof(keyboardled): ", sizeof(keyboardled));
    LOG_DEBUG("sizeof(macros): ", sizeof(macros));
    LOG_DEBUG("sizeof(oef_param_t)*2: ", sizeof(oef_param_t)*2);
  }
#endif

  versionTag t;
  size_t offset;
  EEPROM.begin(EEPROM_SIZE);

  offset = sizeof(primary_profile) + prof * (sizeof(t) + sizeof(km) + sizeof(keyattr));
  EEPROM.get(offset, t);
  if(!checkVersionTag(t)) {
    return false;
  }

  offset += sizeof(t);
  EEPROM.get(offset, km);
  CustomKeymap::validateKeymap(km); // validate and change map data if needed
  offset += sizeof(km);
  EEPROM.get(offset, keyattr);
  MagswStateMachine::validateKeyattr(keyattr);
  offset += sizeof(keyattr);

  offset = sizeof(primary_profile) + PROFILE_MAX * (sizeof(t) + sizeof(km) + sizeof(keyattr));
  EEPROM.get(offset, ledattr);
  F_SystemLED::validateLEDAttr(ledattr);
  offset += sizeof(ledattr);

  EEPROM.get(offset, keyboardled);
  offset += sizeof(keyboardled);

  EEPROM.get(offset, macros);
  for(uint8_t i = 0; i < DEFS_NMACROS; i++) {
    F_KeyMacro::validate(macros[i]);
  }
  offset += sizeof(macros);

  EEPROM.get(offset, tpoef_left);
  if(!F_Touchpad::validateOEFParam(tpoef_left)) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Invalid Touchpad OEF param for left touchpad, reset to default.");
      LOG_DEBUG("fd", tpoef_left.fd, ", fmin", tpoef_left.fmin, ", beta", tpoef_left.beta);
    }
#endif
    tpoef_left = default_tpoef;
  }
  offset += sizeof(tpoef_left);
  EEPROM.get(offset, tpoef_right);
  if(!F_Touchpad::validateOEFParam(tpoef_right)) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Invalid Touchpad OEF param for right touchpad, reset to default.");
      LOG_DEBUG("fd", tpoef_right.fd, ", fmin", tpoef_right.fmin, ", beta", tpoef_right.beta);
    }
#endif
    tpoef_right = default_tpoef;
  }
  offset += sizeof(tpoef_right);

  EEPROM.end();

  if(dest == PROFILE_ACTIVE) {
    if(origin == PROFILE_PRIMARY) loaded_profile = primary_profile;
    else if(origin < PROFILE_MAX) loaded_profile = prof;
    ctxkeymap.keymap.optimizeKeymap();
  }
  // in other cases don't set profile because loadConfiguration() is also used to just check the origin content.

#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("Load from origin ", origin, ", dest ", dest, ", primary_profile ", primary_profile, ".  EEPROM size: ", offset);
  }
#endif
  char buf[] = "Profile 0";
  buf[8] = '0' + prof;
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Loaded Keymap", buf);
  return true;
}

Keymap &Config::fetchKeymap2tmp(bool &r, uint8_t prof, uint8_t layout) {
  switch(prof) {
    case PROFILE_ACTIVE:
      tmp_keymap = active_keymap;
      r = true;
      break;
    case PROFILE_DEFAULT:
      if(!CustomKeymap::validateOSLayout(layout)) layout = OSLAYOUT_US;
      tmp_keymap = default_keymap[layout];
      r = true;
      break;
    default:
      if(!loadConfiguration(prof, 0, tmp_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right)) {
        tmp_keymap.layout = OSLAYOUT_US;
        r = false;
      } else {
        r = true;
      }
      break;
  }
  return tmp_keymap;
}

KeyswAttributes (&Config::fetchKeyAttributes2tmp(bool &r, uint8_t prof))[DEFS_NMAGKEYS] {
  switch(prof) {
    case PROFILE_ACTIVE:
      memcpy(tmp_magkeyattr, active_magkeyattr, sizeof(active_magkeyattr));
      r = true;
      break;
    case PROFILE_DEFAULT:
      memcpy(tmp_magkeyattr, default_magkeyattr, sizeof(default_magkeyattr));
      r = true;
      break;
    default:
      if(!loadConfiguration(prof, 0, tmp_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right)) {
        r = false;
      } else {
        r = true;
      }
      break;
  }
  return tmp_magkeyattr;
}

ConfigKeyboardLED &Config::fetchConfigKeyboardLED2tmp(bool &r, uint8_t prof) {
  switch(prof) {
    case PROFILE_ACTIVE:
      tmp_keyboardled = active_keyboardled;
      r = true;
      break;
    case PROFILE_DEFAULT:
      tmp_keyboardled = default_keyboardled;
      r = true;
      break;
    default:
      if(!loadConfiguration(prof, 0, tmp_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right)) {
        // use prof number instead of 0, because to retrieve led info with valid saved profile.
        r = false;
      } else {
        r = true;
      }
      break;
  }
  return tmp_keyboardled;
}

I2CLEDAttributes &Config::fetchLEDAttributes2tmp(bool &r, uint8_t prof) {
  switch(prof) {
    case PROFILE_ACTIVE:
      tmp_ledattr = active_ledattr;
      r = true;
      break;
    case PROFILE_DEFAULT:
      tmp_ledattr = default_ledattr;
      r = true;
      break;
    default:
      if(!loadConfiguration(prof, 0, tmp_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right)) {
        r = false;
      } else {
        r = true;
      }
      break;
  }
  return tmp_ledattr;
}

#if defined(DEBUG)
void Config::showconf(Stream *s, const char *title, Keymap &km)
{
  s->print(title);
  s->print(" keymap: layout ");
  s->print(km.layout == OSLAYOUT_US ? "US" : "JP");
  s->print("\n");
  s->print("Function x ");
  s->print(KEYMAPTABLE_MAX);
  s->print(" layers\n");

  for(uint8_t i = 0; i < DEFS_NMAGKEYS; i++) {
    s->print("KEY ");
    if(i < 10) s->print(' ');
    s->print(i);
    s->print(" : ");

    for(uint8_t j = 0; j < KEYMAPTABLE_MAX; j++) {
      const char *disp = ctxkeymap.keymap.getFunctionDisp(km.ftable[j][i]);
      if(disp == nullptr) {
        disp = "null";
      }
      uint8_t len = strlen(disp);
      s->print(disp);
      s->print(" / ");
      len++;
      s->print(km.ftable[j][i]);
      if(km.ftable[j][i] < 10) len += 1;
      else if(km.ftable[j][i] < 100) len += 2;
      else len += 3;
      while(len++ < 24) { s->print(' '); }
    }
    s->print('\n');
    if(i % 12 == 11 || i == DEFS_NMAGKEYS - 1) s->print('\n');
  }

  uint8_t j = DEFS_NMAGKEYS;
  while(j < sizeof(CustomKeymap::ksm)/sizeof(CustomKeymap::ksm[0])) {
    s->print("KEY ");
    if(j < 10) s->print(' ');
    s->print(j);
    s->print(" : ");
    s->print(CustomKeymap::ksm[j]->getClassName());
    s->print('\n');
    j++;
  }

  s->print("\nFunction Units");
  s->print("\nKEY\tSTATE\tLAYER\tFUNC\n");
  for(uint8_t i = 0; i < sizeof(km.funit) / sizeof(km.funit[0]); i++) {
    FunctionUnit &fu = km.funit[i];
    if(fu.key < 10) s->print(' ');
    s->print(fu.key);
    s->print(" : ");
    const char *statename = KeyswStateMachine::getStateName(fu.state);
    if(statename == nullptr) statename = "null";
    s->print(statename);
    uint8_t spc = 15 - strlen(statename);
    while(spc--) s->print(' ');
    s->print(fu.layer);
    s->print("   ");
    const char *disp = ctxkeymap.keymap.getFunctionDisp(fu.func);
    if(disp == nullptr) disp = "null";
    s->print(disp);
    s->print(" / ");
    s->print(fu.func);
    s->print('\n');
  }

  s->print("\nStatic Units\n");
  for(uint8_t i = 0; i < CustomKeymap::num_funit_common; i++) {
    const FunctionUnit &fu = CustomKeymap::funit_common[i];
    if(fu.key < 10) s->print(' ');
    s->print(fu.key);
    s->print(" : ");
    const char *statename = KeyswStateMachine::getStateName(fu.state);
    if(statename == nullptr) statename = "null";
    s->print(statename);
    uint8_t spc = 15 - strlen(statename);
    while(spc--) s->print(' ');
    s->print(fu.layer);
    s->print("   ");
    const char *disp = ctxkeymap.keymap.getFunctionDisp(fu.func);
    if(disp == nullptr) disp = "null";
    s->print(disp);
    s->print(" / ");
    s->print(fu.func);
    s->print('\n');
  }

  s->print("\nTap Advanced: ");
  s->print(km.tapadv_enable ? "On" : "Off");
  s->print("\n\n");
}

void Config::showconf(Stream *s, const char *title, KeyswAttributes (&keyattr)[DEFS_NMAGKEYS])
{
  s->print(title);
  s->print(" keyattr\n");
  s->print("Fixed parameters: dr_hL ");
  s->print(keyattr[0].duration_holdL);
  s->print(", dr_hLL ");
  s->print(keyattr[0].duration_holdLL);
  s->print("\n\n");

  s->print("Changeable parameters\n\tth_pp\tth_p\tth_pp2r\tth_p2r\tdr_hold\tdr_pp\tctrl\n");
  for(int i = 0; i < DEFS_NMAGKEYS; i++) {
    s->print("KEY ");
    if(i < 10) s->print(' ');
    s->print(i);
    s->print(" : ");
    s->print(keyattr[i].thresh_partial_press);
    s->print("\t");
    s->print(keyattr[i].thresh_press);
    s->print("\t");
    s->print(keyattr[i].thresh_partial2release);
    s->print("\t");
    s->print(keyattr[i].thresh_press2release);
    s->print("\t");
    s->print(keyattr[i].duration_hold);
    s->print("\t");
    s->print(keyattr[i].duration_partial_press);
    s->print("\t");
    s->print(keyattr[i].control);
    s->print('\n');
  }
  s->print("\n\n");
}

void Config::showconf(Stream *s, const char *title,
  I2CLEDAttributes &ledattr, ConfigKeyboardLED &keyboardled, MacroSequence (&macros)[DEFS_NMACROS],
  oef_param_t &tpoef_left, oef_param_t &tpoef_right)
{
  s->print(title);
  s->print(" key ledattr\n");
  s->print("release\tpartial\tpress\talert\tBkLight\tbright\tBacklightEnable\n");
  s->print(ledattr.led_ptn_release);
  s->print('\t');
  s->print(ledattr.led_ptn_partialpress);
  s->print('\t');
  s->print(ledattr.led_ptn_press);
  s->print('\t');
  s->print(ledattr.led_ptn_alert);
  s->print('\t');
  s->print(ledattr.led_backlight);
  s->print('\t');
  s->print(ledattr.led_brightness);
  s->print('\t');
  s->print(ledattr.backlight_enable);
  s->print("\n\n");

  s->print(title);
  s->print(" sys led color\n");
  s->print("cal 0x");
  s->print(keyboardled.color[LEDTYPE_CALIBRATION], HEX);
  s->print(", sys 0x");
  s->print(keyboardled.color[LEDTYPE_SYSTEM], HEX);
  s->print(", mcu 0x");
  s->print(keyboardled.color[LEDTYPE_MCU], HEX);
  s->print(", key 0x");
  s->print(keyboardled.color[LEDTYPE_KEY], HEX);
  s->print(", numlock 0x");
  s->print(keyboardled.color[LEDTYPE_NUMLOCK], HEX);
  s->print(", capslock 0x");
  s->print(keyboardled.color[LEDTYPE_CAPSLOCK], HEX);
  s->print(", scrolllock 0x");
  s->print(keyboardled.color[LEDTYPE_SCROLLLOCK], HEX);
  s->print("\n\n");

  s->print(title);
  s->print(" macros\n");
  for(uint8_t i = 0; i < DEFS_NMACROS; i++) {
    MacroSequence &ms = macros[i];
    s->print("MACRO ");
    s->print(i);
    s->print(",  ");
    s->print(ms.num);
    s->print(" entries\n");
    uint8_t j = 0;
    while(j < ms.num && j < MACRO_MAXENTRIES) {
      s->print("  entry ");
      s->print(j);
      s->print(", dur ");
      s->print(ms.entry[j].duration);
      s->print(", simul ");
      s->print(ms.entry[j].simultaneous);
      s->print(", funcs ");
      s->print(ms.entry[j].num);
      s->print(" : ");
      uint8_t k = 0;
      while(k < ms.entry[j].num && k < MACRO_MAXFUNCS) {
        s->print(ms.entry[j].fcode[k]);
        s->print(' ');
        k++;
      }
      s->print('\n');
      j++;
    }
    s->print("\n");
  }

  s->print(title);
  s->print(" Touchpad OEF Param\n");
  s->print(" Left fd ");
  s->print(tpoef_left.fd);
  s->print(", fmin ");
  s->print(tpoef_left.fmin);
  s->print(", beta ");
  s->print(tpoef_left.beta);
  s->print("\n Right fd ");
  s->print(tpoef_right.fd);
  s->print(", fmin ");
  s->print(tpoef_right.fmin);
  s->print(", beta ");
  s->print(tpoef_right.beta);
  s->print("\n\n");
}


const char *Config::kmCmd = "keymap";
const char *Config::kmSyntax = "keymap a(ctive) | p(rimary) | 0-3(saved) | d(def)  [m(agattr) | c(ommon)]\n" \
  "keymap l(oad) [p(rimary) | 0-3 | d(efault) [k(eymap) | m(agattr) | c(common)]]\n" \
  "keymap s(ave) [p(rimary) | 0-3] | [P 0-3]\n" \
  "keymap c(lear)";
const char *Config::kmDescr = "Operate profiles of keymap, MAG attrs, LED attrs, and Macros";
int Config::km(Stream *s, uint8_t ac, const char **av) {
  if(ac == 0) return -1;

  switch(*av[0]) {
    case 'a':
    case 'p':
    case '0':
    case '1':
    case '2':
    case '3':
    case 'd':
      return kmShow(s, ac, av);

    case 'l':
      return kmLoad(s, ac - 1, av + 1);

    case 's':
      return kmSave(s, ac - 1, av + 1);

    case 'c':
      return kmClear(s, ac - 1, av + 1);

    default:
      return -1;
  }
}

// a(ctive) | p(rimary) | 0-3(saved) | d(def)  [m(agattr) | c(ommon)]
int Config::kmShow(Stream *s, uint8_t ac, const char **av) {
  uint8_t origin = PROFILE_ACTIVE;
  uint8_t type = 0; // 0=keymap, 1=magattr, 2=ledattr
  if(ac > 0) {
    switch(*av[0]) {
      case 'a':
        break;
      case 'p':
        origin = primary_profile;
        break;
      case '0':
      case '1':
      case '2':
      case '3':
        origin = *av[0] - '0';
        if(origin >= PROFILE_MAX) origin = 0;
        break;
      case 'd':
        origin = PROFILE_DEFAULT;
        break;
      default:
        return -1;
    }
  }
  if(ac > 1) {
    switch(*av[1]) {
      case 'm':
        type = 1;
        break;
      case 'c':
        type = 2;
        break;
      default:
        return -1;
    }
  }

  bool r;
  s->print("Primary Profile: ");
  s->print(primary_profile);
  s->print('\n');
  switch(origin) {
    case PROFILE_ACTIVE:
      if(type == 0) showconf(s, "Active", active_keymap);
      else if(type == 1) showconf(s, "Active", active_magkeyattr);
      else showconf(s, "Active", active_ledattr, active_keyboardled, active_macros, active_tpoef_left, active_tpoef_right);
      break;

    case PROFILE_DEFAULT:
      r = loadConfiguration(PROFILE_DEFAULT, 0, tmp_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right);
      if(!r) break;
      if(type == 0) showconf(s, "Default", tmp_keymap);
      else if(type == 1) showconf(s, "Default", tmp_magkeyattr);
      else showconf(s, "Default", tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right);
      break;

    default:
      r = loadConfiguration(origin, 0, tmp_keymap, tmp_magkeyattr, tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right);
      if(!r) break;
      s->print("Profile ");
      s->print(origin);
      s->print('\n');
      if(type == 0) showconf(s, "Profile", tmp_keymap);
      else if(type == 1) showconf(s, "Profile", tmp_magkeyattr);
      else showconf(s, "Active", tmp_ledattr, tmp_keyboardled, tmp_macros, tmp_tpoef_left, tmp_tpoef_right);
  }
  return 1;
}

// [p(rimary) | 0-3 | d(efault) [k(eymap) | m(agattr) | c(common)]]
int Config::kmLoad(Stream *s, uint8_t ac, const char **av) {
  uint8_t origin = PROFILE_PRIMARY;
  uint8_t type = KM_ALL;

  if(ac > 0) {
    switch(*av[0]) {
      case '0':
      case '1':
      case '2':
      case '3':
        origin = *av[0] - '0';
        if(origin >= PROFILE_MAX) {
          LOG_ERROR("Invalid profile number: ", origin);
          return -1;
        }
        break;
  
      case 'p':
        origin = PROFILE_PRIMARY;
        break;
  
      case 'd':
        origin = PROFILE_DEFAULT;
        break;
  
      default:
        return -1;
    }

    if(ac > 1) {
      switch(*av[1]) {
        case 'k':
          type = KM_KEYMAP;
          break;
        case 'm':
          type = KM_MAGATTR;
          break;
        case 'c':
          type = KM_COMMON;
          break;
        default:
          return -1;
      }
    }
  }

  if(loadConfiguration(origin, type)) {
    return 1;
  }

  return -1;
}

// [p(rimary) | 0-3] | [P 0-3]
int Config::kmSave(Stream *s, uint8_t ac, const char **av) {
  uint8_t origin = PROFILE_PRIMARY;
  if(ac > 0) {
    switch(*av[0]) {
      case '0':
      case '1':
      case '2':
      case '3':
        origin = *av[0] - '0';
        if(origin >= PROFILE_MAX) {
          LOG_ERROR("Invalid profile number: ", origin);
          return -1;
        }
        break;

      case 'p':
        break;

      case 'P':
        if(ac < 2) return -1;
        origin = strtol(av[1], nullptr, 0);
        if(origin >= PROFILE_MAX) {
          LOG_ERROR("Invalid profile number: ", origin);
          return -1;
        }
        savePrimaryProfileNum(origin);
        s->print("# save primary profile number ");
        s->print(origin);
        return 1;

      default:
        return -1;
    }
  }

  Config::saveConfiguration(origin);
  s->print("# save keymap configuration to EEPROM\n");
  return 1;
}

//
int Config::kmClear(Stream *s, uint8_t ac, const char **av) {
  EEPROM.begin(EEPROM_SIZE);
  for(size_t i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xff);
  }
  EEPROM.end();
  return 1;
}

bool Config::checkVersionTag(versionTag t) {
  if(memcmp(&t, &vtag, sizeof(versionTag)) == 0) return true;
  LOG_ERROR("Config version mismatch: ", DebugLogBase::HEX,
    (int)t.b[0], (int)t.b[1], (int)t.b[2], " != ",
    (int)vtag.b[0], (int)vtag.b[1], (int)vtag.b[2]);
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Keymap version", "Mismatch");
  return false;
}

#endif

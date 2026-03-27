#pragma once
#include <Arduino.h>
#include "controller.h"
#include "sysutils.h"
#include "i2c_control.h"
#include "mcu.h"
#include "custom_keymap.h"
#include "keysw_statemachine.h"
#include "functions.h"

#define EEPROM_SIZE 4096

enum ProfileType : uint8_t {
  PROFILE_0 = 0,
  PROFILE_1,
  PROFILE_2,
  PROFILE_3,
  PROFILE_MAX,
  PROFILE_PRIMARY = 124,
  PROFILE_DEFAULT,
  PROFILE_ACTIVE,
};

enum KeymapType : uint8_t {
  KM_ALL = 0,
  KM_KEYMAP,
  KM_MAGATTR,
  KM_COMMON,
  KM_MAX,
};

struct ConfigKeyboardLED {
  uint32_t color[LEDTYPE_MAX];
}; 

class Config {
public:
  Config() {}
  static bool loadConfiguration(uint8_t origin, uint8_t type = 0);
  static bool loadConfiguration(uint8_t origin, uint8_t dest,
    Keymap &keymap, KeyswAttributes (&keyattr)[DEFS_NMAGKEYS],
    I2CLEDAttributes &ledattr, ConfigKeyboardLED &keyboardled, MacroSequence (&macro)[DEFS_NMACROS],
    oef_param_t &tpoef_left, oef_param_t &tpoef_right);
  static bool saveConfiguration(uint8_t prof);
  static bool savePrimaryProfileNum(uint8_t prof);
  static bool saveConfigLED();
  static bool saveTPOEFParam();
  static uint8_t getPrimaryProfileNum() { return(primary_profile); }
  static bool setPrimaryProfileNum(uint8_t prof) { if(prof < PROFILE_MAX) { primary_profile = prof; return true; } return false; }
  static char getPrimaryProfileChar() { return('0' + primary_profile); }
  static char getLoadedProfileNum() { return(loaded_profile); }
  static char getLoadedProfileChar() { return(loaded_profile == PROFILE_DEFAULT ? 'D' : '0' + loaded_profile); }
  static Keymap &getActiveKeymap() { return active_keymap; }
  static ConfigKeyboardLED &getActiveKeyboardLED() { return active_keyboardled; }
  static I2CLEDAttributes &getActiveLEDAttributes() { return active_ledattr; }
  static MacroSequence &getActiveMacro(uint8_t index) { return active_macros[(index >= DEFS_NMACROS) ? 0 : index]; }
  static bool validateProfileAll(uint8_t prof) { return (prof == PROFILE_ACTIVE || prof == PROFILE_DEFAULT || prof == PROFILE_PRIMARY || prof < PROFILE_MAX); }
  static Keymap &fetchKeymap2tmp(bool &r, uint8_t prof, uint8_t layout = OSLAYOUT_US);
  static KeyswAttributes (&fetchKeyAttributes2tmp(bool &r, uint8_t prof))[DEFS_NMAGKEYS];
  static ConfigKeyboardLED &fetchConfigKeyboardLED2tmp(bool &r, uint8_t prof);
  static I2CLEDAttributes &fetchLEDAttributes2tmp(bool &r, uint8_t prof);
  static bool setI2CLEDAttributesByCopy(I2CLEDAttributes &ledattr) {
    KeySensorMCU::setLEDAttributesAll(ledattr);
    active_ledattr = ledattr;
    return true; 
  }

#if defined(DEBUG)
  static void showconf(Stream *s, const char *title, Keymap &keymap);
  static void showconf(Stream *s, const char *title, KeyswAttributes (&keyattr)[DEFS_NMAGKEYS]);
  static void showconf(Stream *s, const char *title,
    I2CLEDAttributes &ledattr, ConfigKeyboardLED &keyboardled, MacroSequence (&macros)[DEFS_NMACROS],
    oef_param_t &tpoef_left, oef_param_t &tpoef_right);

  static const char *kmCmd;
  static const char *kmSyntax;
  static const char *kmDescr;
  static int km(Stream *s, uint8_t ac, const char **av);
  static int kmShow(Stream *s, uint8_t ac, const char **av);
  static int kmLoad(Stream *s, uint8_t ac, const char **av);
  static int kmSave(Stream *s, uint8_t ac, const char **av);
  static int kmClear(Stream *s, uint8_t ac, const char **av);

#endif

private:
  struct versionTag {
    uint8_t b[3];
  };
  static constexpr uint8_t CONFVERSION = 1; // inclement when the structure is changed
    /*
    1: initial version
    2: added I2CLEDAttributes.led_backlight, backlight_enable, MacroSequence.simultaneous, keymap.tapadv_enable
    */
  static constexpr struct versionTag vtag = {0x3c, CONFVERSION, 0x69};
  static bool checkVersionTag(versionTag t);
  inline static uint8_t loaded_profile = 0; // profile loaded from EEPROM to the active tables
  inline static uint8_t primary_profile = 0; // profile to be used when bootup. saved in EEPROM(0).

  static Keymap default_keymap[OSLAYOUT_MAX];
  static KeyswAttributes default_magkeyattr[DEFS_NMAGKEYS];
  static I2CLEDAttributes default_ledattr;
  static ConfigKeyboardLED default_keyboardled;
  static MacroSequence default_macros[DEFS_NMACROS];
  static const oef_param_t &default_tpoef;

  inline static Keymap active_keymap = {};
  static KeyswAttributes (&active_magkeyattr)[DEFS_NMAGKEYS];
  inline static I2CLEDAttributes active_ledattr = {};
  inline static ConfigKeyboardLED active_keyboardled = {};
  inline static MacroSequence active_macros[DEFS_NMACROS] = {};
  static oef_param_t &active_tpoef_left;
  static oef_param_t &active_tpoef_right;

  inline static Keymap tmp_keymap = {};
  inline static KeyswAttributes tmp_magkeyattr[DEFS_NMAGKEYS] = {};
  inline static I2CLEDAttributes tmp_ledattr = {};
  inline static ConfigKeyboardLED tmp_keyboardled = {};
  inline static MacroSequence tmp_macros[DEFS_NMACROS] = {};
  inline static oef_param_t tmp_tpoef_left = {};
  inline static oef_param_t tmp_tpoef_right = {};
};

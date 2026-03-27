#pragma once

#include "i2c_control.h"


#define _STRINGIFY_(x) #x
#define STRINGIFY(x) _STRINGIFY_(x)



#if !defined(MB_MAJOR) || !defined(MB_MINOR) || !defined(MB_PATCH)
#error "MainBoard VERSION is not defined"
#endif
// Main Firmware version 1.0.0
#define MF_MAJOR 1
#define MF_MINOR 0
#define MF_PATCH 0
#define MF_VERSION_STR STRINGIFY(MF_MAJOR) "." STRINGIFY(MF_MINOR) "." STRINGIFY(MF_PATCH)
#define MB_VERSION_STR STRINGIFY(MB_MAJOR) "." STRINGIFY(MB_MINOR) "." STRINGIFY(MB_PATCH)


// Supported Boards and Functions
#if MB_MAJOR == 0
#if defined(ARDUINO_SEEED_XIAO_RP2040)
#define SUPPORT_XIAO_RP2040_0
#elif defined(ARDUINO_SEEED_XIAO_RP2350)
#define SUPPORT_XIAO_RP2350_0
#endif
#define SUPPORT_TINYMOUSE
#define SUPPORT_LOADSWITCH_CH217K
#define SUPPORT_MCU_012543
#define SUPPORT_LOCKLED // CapsLock, NumLock, ScrollLock LED control
#define DISABLE_BUILTIN_RGBLED
#define DEFS_SYS_RGB_TYPE NEO_GRB
#define DEFS_USBDEVICE_PRODUCTSTRING "MKB"
#define DEFS_HID_BOOTKEYBOARD_NAME "MKB"
#define DEFS_HID_COMPOSITE_NAME "MKB(O_O)"
#define DEFS_USBDEVICE_MANUFACTURER "zbn10 WorkShop"
#define DEFS_OLED_TEXT1 "MKB   \\(o_o)/"
#define DEFS_OLED_TEXT2 "Magsw Keyboard"

#elif MB_MAJOR == 1
#if defined(ARDUINO_SEEED_XIAO_RP2040)
#define SUPPORT_XIAO_RP2040_1
#elif defined(ARDUINO_SEEED_XIAO_RP2350)
#define SUPPORT_XIAO_RP2350_1
#endif
#define SUPPORT_PHOTOIC_INFRARED
#define SUPPORT_LOADSWITCH_TCK107AF
#define SUPPORT_MCU_012543
#define SUPPORT_TOUCHPAD 1 // version 0.1
#define DISABLE_BUILTIN_RGBLED
#define DEFS_SYS_RGB_TYPE NEO_GRB
#define DEFS_USBDEVICE_PRODUCTSTRING "MOKA"
#define DEFS_HID_BOOTKEYBOARD_NAME "MOKA"
#define DEFS_HID_COMPOSITE_NAME "MOKA(O_O)"
#define DEFS_USBDEVICE_MANUFACTURER "zbn10 WorkShop"
#define DEFS_OLED_TEXT1 "MOKA  \\(o_o)/"
#define DEFS_OLED_TEXT2 "Magsw Keyboard"

#elif MB_MAJOR == 2
#if defined(ARDUINO_WAVESHARE_RP2040_ZERO)
#define SUPPORT_WAVESHARE_RP2040ZERO_2
#elif defined(ARDUINO_WAVESHARE_RP2350_ZERO)
#define SUPPORT_WAVESHARE_RP2350ZERO_2
#endif
#define SUPPORT_PHOTOIC_INFRARED
#define SUPPORT_LOADSWITCH_TCK107AF
#define SUPPORT_MCU_012345
#define SUPPORT_TOUCHPAD 100 // version 1.0
#define DEFS_SYS_RGB_TYPE NEO_RGB
#define SUPPORT_SYS_RGB_REDUCE_BRIGHTNESS
#define SUPPORT_STAGGERED
#define DEFS_USBDEVICE_PRODUCTSTRING "MOKA"
#define DEFS_HID_BOOTKEYBOARD_NAME "MOKA(O_O)"
#define DEFS_HID_COMPOSITE_NAME "MOKA(O_O)"
#define DEFS_USBDEVICE_MANUFACTURER "zbn10 WorkShop"
#define DEFS_OLED_TEXT1 "MOKA  \\(o_o)/"
#define DEFS_OLED_TEXT2 "Magsw Keyboard"

#else 
#error "MB_MAJOR not supported"
#endif

typedef struct {
  I2CVersionInfo mf_vinfo;
  I2CVersionInfo mb_vinfo_minimum;
  I2CVersionInfo mb_vinfo_maximum;
  I2CVersionInfo mcu_vinfo_minimum;
  I2CVersionInfo mcu_vinfo_maximum;
  I2CVersionInfo tpsm_vinfo_minimum;
  I2CVersionInfo tpsm_vinfo_maximum;
} version_matrix_t;

void disableBuiltinLED();
bool checkVersionCompatibility();
int8_t compareVersionInfo(I2CVersionInfo &v1, I2CVersionInfo &v2); // return -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2

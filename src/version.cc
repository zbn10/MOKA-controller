#include "controller.h"
#include "mcu.h"
#include "keysw_statemachine.h"
#include "version.h"


/*

**Firmware Matrix**

|Main Firmware|Main Board|Keysensor Firmware|Touchpad Firmware|
|:----|:----|:----|:----|
|1.0.0|>= 0.9|1.0.0|1.0.0|


**Hardware Matrix**

|Main Board|Touchpad Board|
|:----|:----|
|0.9|-|
|2.0|1.0|
|2.1|1.1|

*/
#define MB_MAJOR_MIN 0
#define MB_MINOR_MIN 9
#define MB_PATCH_MIN 0
version_matrix_t ver_matrix = {
  {MF_MAJOR, MF_MINOR, MF_PATCH}, // MF_VER
  {MB_MAJOR_MIN, MB_MINOR_MIN, MB_PATCH_MIN}, {255, 255, 255}, // MB_VER Min, MB_VER Max
  {1, 0, 0}, {255, 25, 25}, // MCU_VER Min, MCU_VER Max
  {1, 0, 0}, {255, 25, 25} // TPSM_VER Min, TPSM_VER Max
};


#if (MB_MAJOR * 10000 + MB_MINOR * 100 + MB_PATCH >= MB_MAJOR_MIN * 10000 + MB_MINOR_MIN * 100 + MB_PATCH_MIN)
#pragma message("MF_VER " MF_VERSION_STR " is compatible with MB_VER " MB_VERSION_STR)
#else
#pragma message("MF_VER " MF_VERSION_STR ", MB_VER " MB_VERSION_STR)
#error "NOT compatible"
#endif
/*
#if (MB_MAJOR == 0 && MB_MINOR == 9) || (MB_MAJOR == 1 && MB_MINOR == 0) || (MB_MAJOR == 2 && MB_MINOR == 0) || (MB_MAJOR == 2 && MB_MINOR == 1)
#pragma message("\nFWVER " MF_VERSION_STR " is compatible with HWVER " MB_VERSION_STR)
#else
#error "FWVER " MF_VERSION_STR " is NOT compatible with HWVER " MB_VERSION_STR
#endif
*/


#if defined(DISABLE_BUILTIN_RGBLED)
Adafruit_NeoPixel builtin_rgbled(1, DEFS_PIN_BUILTIN_RGBLED, NEO_GRB + NEO_KHZ800);
#endif


void disableBuiltinLED() {
#if defined(SUPPORT_XIAO_RP2040_0) || defined(SUPPORT_XIAO_RP2040_1)
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  pinMode(17, OUTPUT);
  digitalWrite(17, HIGH);
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
#elif defined(SUPPORT_XIAO_RP2350_0) || defined(SUPPORT_XIAO_RP2350_1)
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
#endif

#if defined(DISABLE_BUILTIN_RGBLED)
  builtin_rgbled.begin();
  builtin_rgbled.setPixelColor(0, 0);
  builtin_rgbled.show();
#endif
}

int8_t compareVersionInfo(I2CVersionInfo &v1, I2CVersionInfo &v2) {
  if(v1.ver_major < v2.ver_major) return -1;
  if(v1.ver_major > v2.ver_major) return 1;
  if(v1.ver_minor < v2.ver_minor) return -1;
  if(v1.ver_minor > v2.ver_minor) return 1;
  if(v1.ver_patch < v2.ver_patch) return -1;
  if(v1.ver_patch > v2.ver_patch) return 1;
  return 0;
}

bool checkVersionCompatibility() {
  I2CVersionInfo vinfo;
  for(uint8_t mcu = 0; mcu < MCU_NMCUS; mcu++) {
    if(ctxmcu.mcu[mcu].getMCUVersion(vinfo) >= 0) {
      if(compareVersionInfo(vinfo, ver_matrix.mcu_vinfo_minimum) < 0) return false;
      if(compareVersionInfo(vinfo, ver_matrix.mcu_vinfo_maximum) > 0) return false;
    }
  }

  if(ctxksm.tpsm_left.getVersion(vinfo) >= 0) {
    if(compareVersionInfo(vinfo, ver_matrix.tpsm_vinfo_minimum) < 0) return false;
    if(compareVersionInfo(vinfo, ver_matrix.tpsm_vinfo_maximum) > 0) return false;
  }
  if(ctxksm.tpsm_right.getVersion(vinfo) >= 0) {
    if(compareVersionInfo(vinfo, ver_matrix.tpsm_vinfo_minimum) < 0) return false;
    if(compareVersionInfo(vinfo, ver_matrix.tpsm_vinfo_maximum) > 0) return false;
  }

  return true;
}

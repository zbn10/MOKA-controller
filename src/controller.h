#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "version.h"


typedef uint8_t FunctionCode_t;

/*
  extra_flags in arduino-cli command line option.
  -DDEBUG
*/

// Common Definitions
#define DEFS_NMAGKEYS               42
#define DEFS_NKEYS                  48
#define DEFS_FUNITS                 30
#define DEFS_NMACROS                6
#define DEFS_HID_KEYBOARD_NKRO_BYTES 12
#define DEFS_PERIODIC_INTERVAL      1000 // ms

#define DEFS_I2CCLOCK               1000000
#define DEFS_I2C2NDCLOCK            400000
#define DEFS_SERIALSPEED            115200


// Pin Definitions
#if defined(SUPPORT_XIAO_RP2040_0)
#define DEFS_PIN_FUNCBTN       D10
#define DEFS_PIN_SYSTEM_LED    D9 // LED0: system status, LED1,2,3: Key status LED if MKB version
#define DEFS_PIN_RESET_MCUS    D8 // for load switch
#define DEFS_PIN_SERIAL_TX     D6
#define DEFS_PIN_SERIAL_RX     D7
#define DEFS_PIN_JOYSTICK_X    A0
#define DEFS_PIN_JOYSTICK_Y    A1
#define DEFS_ANALOGRESOLUTION  10
#define DEFS_PIN_WIREMCU_SDA   D4 // For Wire in default
#define DEFS_PIN_WIREMCU_SCL   D5 // For Wire in default
#define DEFS_PIN_WIRE2ND_SDA   D2 // no defined in default
#define DEFS_PIN_WIRE2ND_SCL   D3 // no defined in default
#define WireMCU                Wire
#define Wire2nd                Wire1
#define DEFS_PIN_BUILTIN_RGBLED 12 // XIAO RP2040 RGB LED

#elif defined(SUPPORT_XIAO_RP2350_0)
#define DEFS_PIN_FUNCBTN       D10
#define DEFS_PIN_SYSTEM_LED    D9 // LED0: system status, LED1,2,3: Key status LED if MKB version
#define DEFS_PIN_RESET_MCUS    D8 // for load switch
#define DEFS_PIN_SERIAL_TX     D6
#define DEFS_PIN_SERIAL_RX     D7
#define DEFS_PIN_JOYSTICK_X    A0
#define DEFS_PIN_JOYSTICK_Y    A1
#define DEFS_ANALOGRESOLUTION  10
#define DEFS_PIN_WIREMCU_SDA   D4 // For Wire1 in default
#define DEFS_PIN_WIREMCU_SCL   D5 // For Wire1 in default
#define DEFS_PIN_WIRE2ND_SDA   D2 // Default is D14 back pad. not actually used
#define DEFS_PIN_WIRE2ND_SCL   D3 // Default is D13 Back Pad. not actually used
#define WireMCU                Wire1
#define Wire2nd                Wire
#define DEFS_PIN_BUILTIN_RGBLED 22 // XIAO RP2350 RGB LED D20

#elif defined(SUPPORT_XIAO_RP2040_1)
#define DEFS_PIN_FUNCBTN       D10
#define DEFS_PIN_SYSTEM_LED    D9 // LED0: system status, LED1,2,3: Key status LED if MKB version
#define DEFS_PIN_RESET_MCUS    D8 // for load switch
#define DEFS_PIN_SERIAL_TX     D6
#define DEFS_PIN_SERIAL_RX     D7
#define DEFS_PIN_INFRARED      D0
#define DEFS_PIN_PHOTOIC       A1
#define DEFS_ANALOGRESOLUTION  12
#define DEFS_PIN_WIREMCU_SDA   D4 // For Wire in default
#define DEFS_PIN_WIREMCU_SCL   D5 // For Wire in default
#define DEFS_PIN_WIRE2ND_SDA   D2 // no defined in default
#define DEFS_PIN_WIRE2ND_SCL   D3 // no defined in default
#define WireMCU                Wire
#define Wire2nd                Wire1
#define DEFS_PIN_BUILTIN_RGBLED 12 // XIAO RP2040 RGB LED

#elif defined(SUPPORT_XIAO_RP2350_1)
#define DEFS_PIN_FUNCBTN       D10
#define DEFS_PIN_SYSTEM_LED    D9 // LED0: system status, LED1,2,3: Key status LED if MKB version
#define DEFS_PIN_RESET_MCUS    D8 // for load switch
#define DEFS_PIN_SERIAL_TX     D6
#define DEFS_PIN_SERIAL_RX     D7
#define DEFS_PIN_INFRARED      D0
#define DEFS_PIN_PHOTOIC       A1
#define DEFS_ANALOGRESOLUTION  12
#define DEFS_PIN_WIREMCU_SDA   D4 // For Wire1 in default
#define DEFS_PIN_WIREMCU_SCL   D5 // For Wire1 in default
#define DEFS_PIN_WIRE2ND_SDA   D2 // Default is D14 back pad. not actually used
#define DEFS_PIN_WIRE2ND_SCL   D3 // Default is D13 Back Pad. not actually used
#define WireMCU                Wire1
#define Wire2nd                Wire
#define DEFS_PIN_BUILTIN_RGBLED 22 // XIAO RP2350 RGB LED D20

#elif defined(SUPPORT_WAVESHARE_RP2040ZERO_2) || defined(SUPPORT_WAVESHARE_RP2350ZERO_2)
#define DEFS_PIN_FUNCBTN       3
#define DEFS_PIN_SYSTEM_LED    16 // built in WS2812 LED
#define DEFS_PIN_RESET_MCUS    2 // for load switch
#define DEFS_PIN_SERIAL_TX     0
#define DEFS_PIN_SERIAL_RX     1
#define DEFS_PIN_INFRARED     26
#define DEFS_PIN_PHOTOIC      27
#define DEFS_ANALOGRESOLUTION 12
#define DEFS_PIN_WIREMCU_SDA   6 // Default is 4
#define DEFS_PIN_WIREMCU_SCL   7 // Default is 5
#define DEFS_PIN_WIRE2ND_SDA   8 // Default is 26
#define DEFS_PIN_WIRE2ND_SCL   9 // Default is 27
#define WireMCU                Wire1
#define Wire2nd                Wire
#define DEFS_PIN_BUILTIN_RGBLED 16

#else
#error "Board not supported"
#endif



/*
  USB version used:
  8 bytes for low speed (1.5MB_Ps)
  64 bytes for full speed (12MB_Ps)
  1024 bytes for high speed (480MB_Ps)
  Set relevant value in wMaxPacketSize
*/

void mainloop_showResults(Stream *s);

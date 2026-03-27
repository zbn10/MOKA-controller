#pragma once
#include <Adafruit_TinyUSB.h>

enum HID : uint8_t {
  HID_NONE = 0x00,
  HID_A = 0x04,
  HID_B = 0x05,
  HID_C = 0x06,
  HID_D = 0x07,
  HID_E = 0x08,
  HID_F = 0x09,
  HID_G = 0x0A,
  HID_H = 0x0B,
  HID_I = 0x0C,
  HID_J = 0x0D,
  HID_K = 0x0E,
  HID_L = 0x0F,
  HID_M = 0x10,
  HID_N = 0x11,
  HID_O = 0x12,
  HID_P = 0x13,
  HID_Q = 0x14,
  HID_R = 0x15,
  HID_S = 0x16,
  HID_T = 0x17,
  HID_U = 0x18,
  HID_V = 0x19,
  HID_W = 0x1A,
  HID_X = 0x1B,
  HID_Y = 0x1C,
  HID_Z = 0x1D,
  HID_ONE = 0x1E, // 1 !
  HID_TWO = 0x1F, // 2 @, 2 "
  HID_THREE = 0x20, // 3 #
  HID_FOUR = 0x21, // 4 $
  HID_FIVE = 0x22, // 5 %
  HID_SIX = 0x23, // 6 ^, 6 &
  HID_SEVEN = 0x24, // 7 &, 7 '
  HID_EIGHT = 0x25, // 8 *, 8 (
  HID_NINE = 0x26, // 9 (, 9 )
  HID_ZERO = 0x27, // 0 ), 0 NONE
  HID_ENTER = 0x28,
  HID_ESC = 0x29,
  HID_BS = 0x2A,
  HID_TAB = 0x2B,
  HID_SPACE = 0x2C,

  HID_MINUS = 0x2D, // - _, - =
  HID_EQUAL = 0x2E, // = +, ^ ~
  HID_BRACKET_LEFT = 0x2F, // [ {, @ `
  HID_BRACKET_RIGHT = 0x30, // ] }, [ {
  HID_BACKSLASH = 0x31, // \ |
  HID_JP_BRACKET_RIGHT = 0x32, // ,] }
  HID_SEMICOLON = 0x33, // ; :, ; +
  HID_QUOTE = 0x34, // ' ", : *
  HID_GRAVE = 0x35, // ` ~, Hankaku Zenkaku

  HID_JP_BACKSLASH = 0x87, // \ _
  HID_JP_KANA = 0x88, // Kana Hiragana
  HID_JP_YEN = 0x89, // YEN |
  HID_JP_CONV = 0x8A, // Convert
  HID_JP_NOCONV = 0x8B, // Non Convert

  HID_COMMA = 0x36, // , <
  HID_PERIOD = 0x37, // . >
  HID_SLASH = 0x38, // / ?
  HID_CAPLK = 0x39,
  HID_F1 = 0x3A,
  HID_F2 = 0x3B,
  HID_F3 = 0x3C,
  HID_F4 = 0x3D,
  HID_F5 = 0x3E,
  HID_F6 = 0x3F,
  HID_F7 = 0x40,
  HID_F8 = 0x41,
  HID_F9 = 0x42,
  HID_F10 = 0x43,
  HID_F11 = 0x44,
  HID_F12 = 0x45,
  HID_PRTSC = 0x46,
  HID_SCRLK = 0x47,
  HID_PAUSE = 0x48,
  HID_INS = 0x49,
  HID_HOME = 0x4A,
  HID_PGUP = 0x4B,
  HID_DEL = 0x4C,
  HID_END = 0x4D,
  HID_PGDN = 0x4E,
  HID_ARW_R = 0x4F,
  HID_ARW_L = 0x50,
  HID_ARW_D = 0x51,
  HID_ARW_U = 0x52,
  HID_NUMLK = 0x53,
  HID_F23 = 0x72,
};

enum MOD : uint8_t {
  MOD_NONE = 0x00,
  MOD_CTR_L = 0x01,
  MOD_SHIFT_L = 0x02,
  MOD_ALT_L = 0x04,
  MOD_GUI_L = 0x08,
  MOD_CTRL_R = 0x10,
  MOD_SHIFT_R = 0x20,
  MOD_ALT_R = 0x40,
  MOD_GUI_R = 0x80
};

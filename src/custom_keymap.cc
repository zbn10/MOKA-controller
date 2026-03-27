#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "keysw_statemachine.h"
#include "custom_keymap.h"
#include "functions.h"
#include "hid.h"
#include "hid_code.h"
#include "sysutils.h"
#include "config.h"


appcontext_keymap_t &appctxKeymap() {
  static appcontext_keymap_t ctx = {
    CustomKeymap(Config::getActiveKeymap()),
    F_SelectLayer(F_NONE, LAYER_0), // loop infinitely
  };
  return ctx;
}


// FunctionUnit CustomKeymap::funit_common[] -> defined in config.cc

KeyswStateMachine *CustomKeymap::ksm[] = {
  // row 0
  &ctxmagsw.msm[0], &ctxmagsw.msm[1], &ctxmagsw.msm[2], &ctxmagsw.msm[3], &ctxmagsw.msm[4], &ctxmagsw.msm[5],
  &ctxmagsw.msm[6], &ctxmagsw.msm[7], &ctxmagsw.msm[8], &ctxmagsw.msm[9], &ctxmagsw.msm[10], &ctxmagsw.msm[11],
  // row 1
  &ctxmagsw.msm[12], &ctxmagsw.msm[13], &ctxmagsw.msm[14], &ctxmagsw.msm[15], &ctxmagsw.msm[16], &ctxmagsw.msm[17],
  &ctxmagsw.msm[18], &ctxmagsw.msm[19], &ctxmagsw.msm[20], &ctxmagsw.msm[21], &ctxmagsw.msm[22], &ctxmagsw.msm[23],
  // row 2
  &ctxmagsw.msm[24], &ctxmagsw.msm[25], &ctxmagsw.msm[26], &ctxmagsw.msm[27], &ctxmagsw.msm[28], &ctxmagsw.msm[29],
  &ctxmagsw.msm[30], &ctxmagsw.msm[31], &ctxmagsw.msm[32], &ctxmagsw.msm[33], &ctxmagsw.msm[34], &ctxmagsw.msm[35],
  // row 3
  &ctxmagsw.msm[36], &ctxmagsw.msm[37], &ctxmagsw.msm[38], &ctxmagsw.msm[39], &ctxmagsw.msm[40], &ctxmagsw.msm[41],
  // other keys
  &ctxksm.tact_sw, // 42

#if defined(SUPPORT_TINYMOUSE)
  &ctxksm.tmouse, // 43
#else
  &ctxksm.dummy_sw, //43
#endif

#if defined(SUPPORT_TOUCHPAD)
  &ctxksm.tpsm_left, //44
  &ctxksm.tpsm_right, //45
#else
  &ctxksm.dummy_sw, //44
  &ctxksm.dummy_sw, //45
#endif

#if defined(SUPPORT_PHOTOIC_INFRARED)
  &ctxksm.photoic_sw, // 46
  &ctxksm.infrared_sw, // 47
#else
  &ctxksm.dummy_sw, //46
  &ctxksm.dummy_sw, //47
#endif
};
static_assert(sizeof(CustomKeymap::ksm) / sizeof(CustomKeymap::ksm[0]) == DEFS_NKEYS,
  "CustomKeymap::ksm[] != DEFS_NKEYS. Check initialization list or DEFS_NKEYS value.");

F_None f_none;

F_Key f_key_bs(F_BS, HID_BS, MOD_NONE);
F_Key f_key_tab(F_TAB, HID_TAB, MOD_NONE);
F_Key f_key_enter(F_ENTER, HID_ENTER, MOD_NONE);
F_Key f_key_esc(F_ESC, HID_ESC, MOD_NONE);
F_Key f_key_space(F_SPACE, HID_SPACE, MOD_NONE);
F_Key f_key_excl(F_EXCL, HID_ONE, MOD_SHIFT_L);
F_Key f_key_dquote(F_DQUOTE, HID_QUOTE, MOD_SHIFT_L);
F_Key f_key_hash(F_HASH, HID_THREE, MOD_SHIFT_L);
F_Key f_key_dollar(F_DOLLAR, HID_FOUR, MOD_SHIFT_L);
F_Key f_key_percent(F_PERCENT, HID_FIVE, MOD_SHIFT_L);
F_Key f_key_ampersand(F_AMPERSAND, HID_SEVEN, MOD_SHIFT_L);
F_Key f_key_quote(F_QUOTE, HID_QUOTE, MOD_NONE);
F_Key f_key_paren_left(F_PAREN_LEFT, HID_NINE, MOD_SHIFT_L);
F_Key f_key_paren_right(F_PAREN_RIGHT, HID_ZERO, MOD_SHIFT_L);
F_Key f_key_asterisk(F_ASTERISK, HID_EIGHT, MOD_SHIFT_L);
F_Key f_key_plus(F_PLUS, HID_EQUAL, MOD_SHIFT_L);
F_Key f_key_comma(F_COMMA, HID_COMMA, MOD_NONE);
F_Key f_key_minus(F_MINUS, HID_MINUS, MOD_NONE);
F_Key f_key_period(F_PERIOD, HID_PERIOD, MOD_NONE);
F_Key f_key_slash(F_SLASH, HID_SLASH, MOD_NONE);
F_Key f_key_0(F_0, HID_ZERO, MOD_NONE);
F_Key f_key_1(F_1, HID_ONE, MOD_NONE);
F_Key f_key_2(F_2, HID_TWO, MOD_NONE);
F_Key f_key_3(F_3, HID_THREE, MOD_NONE);
F_Key f_key_4(F_4, HID_FOUR, MOD_NONE);
F_Key f_key_5(F_5, HID_FIVE, MOD_NONE);
F_Key f_key_6(F_6, HID_SIX, MOD_NONE);
F_Key f_key_7(F_7, HID_SEVEN, MOD_NONE);
F_Key f_key_8(F_8, HID_EIGHT, MOD_NONE);
F_Key f_key_9(F_9, HID_NINE, MOD_NONE);
F_Key f_key_colon(F_COLON, HID_SEMICOLON, MOD_SHIFT_L);
F_Key f_key_semicolon(F_SEMICOLON, HID_SEMICOLON, MOD_NONE);
F_Key f_key_angle_left(F_ANGLE_LEFT, HID_COMMA, MOD_SHIFT_L);
F_Key f_key_equal(F_EQUAL, HID_EQUAL, MOD_NONE);
F_Key f_key_angle_right(F_ANGLE_RIGHT, HID_PERIOD, MOD_SHIFT_L);
F_Key f_key_question(F_QUESTION, HID_SLASH, MOD_SHIFT_L);
F_Key f_key_at(F_AT, HID_TWO, MOD_SHIFT_L);
F_Key f_key_a(F_a, HID_A, MOD_NONE);
F_Key f_key_b(F_b, HID_B, MOD_NONE);
F_Key f_key_c(F_c, HID_C, MOD_NONE);
F_Key f_key_d(F_d, HID_D, MOD_NONE);
F_Key f_key_e(F_e, HID_E, MOD_NONE);
F_Key f_key_f(F_f, HID_F, MOD_NONE);
F_Key f_key_g(F_g, HID_G, MOD_NONE);
F_Key f_key_h(F_h, HID_H, MOD_NONE);
F_Key f_key_i(F_i, HID_I, MOD_NONE);
F_Key f_key_j(F_j, HID_J, MOD_NONE);
F_Key f_key_k(F_k, HID_K, MOD_NONE);
F_Key f_key_l(F_l, HID_L, MOD_NONE);
F_Key f_key_m(F_m, HID_M, MOD_NONE);
F_Key f_key_n(F_n, HID_N, MOD_NONE);
F_Key f_key_o(F_o, HID_O, MOD_NONE);
F_Key f_key_p(F_p, HID_P, MOD_NONE);
F_Key f_key_q(F_q, HID_Q, MOD_NONE);
F_Key f_key_r(F_r, HID_R, MOD_NONE);
F_Key f_key_s(F_s, HID_S, MOD_NONE);
F_Key f_key_t(F_t, HID_T, MOD_NONE);
F_Key f_key_u(F_u, HID_U, MOD_NONE);
F_Key f_key_v(F_v, HID_V, MOD_NONE);
F_Key f_key_w(F_w, HID_W, MOD_NONE);
F_Key f_key_x(F_x, HID_X, MOD_NONE);
F_Key f_key_y(F_y, HID_Y, MOD_NONE);
F_Key f_key_z(F_z, HID_Z, MOD_NONE);
F_Key f_key_A(F_A, HID_A, MOD_SHIFT_L);
F_Key f_key_B(F_B, HID_B, MOD_SHIFT_L);
F_Key f_key_C(F_C, HID_C, MOD_SHIFT_L);
F_Key f_key_D(F_D, HID_D, MOD_SHIFT_L);
F_Key f_key_E(F_E, HID_E, MOD_SHIFT_L);
F_Key f_key_F(F_F, HID_F, MOD_SHIFT_L);
F_Key f_key_G(F_G, HID_G, MOD_SHIFT_L);
F_Key f_key_H(F_H, HID_H, MOD_SHIFT_L);
F_Key f_key_I(F_I, HID_I, MOD_SHIFT_L);
F_Key f_key_J(F_J, HID_J, MOD_SHIFT_L);
F_Key f_key_K(F_K, HID_K, MOD_SHIFT_L);
F_Key f_key_L(F_L, HID_L, MOD_SHIFT_L);
F_Key f_key_M(F_M, HID_M, MOD_SHIFT_L);
F_Key f_key_N(F_N, HID_N, MOD_SHIFT_L);
F_Key f_key_O(F_O, HID_O, MOD_SHIFT_L);
F_Key f_key_P(F_P, HID_P, MOD_SHIFT_L);
F_Key f_key_Q(F_Q, HID_Q, MOD_SHIFT_L);
F_Key f_key_R(F_R, HID_R, MOD_SHIFT_L);
F_Key f_key_S(F_S, HID_S, MOD_SHIFT_L);
F_Key f_key_T(F_T, HID_T, MOD_SHIFT_L);
F_Key f_key_U(F_U, HID_U, MOD_SHIFT_L);
F_Key f_key_V(F_V, HID_V, MOD_SHIFT_L);
F_Key f_key_W(F_W, HID_W, MOD_SHIFT_L);
F_Key f_key_X(F_X, HID_X, MOD_SHIFT_L);
F_Key f_key_Y(F_Y, HID_Y, MOD_SHIFT_L);
F_Key f_key_Z(F_Z, HID_Z, MOD_SHIFT_L);
F_Key f_key_bracket_left(F_BRACKET_LEFT, HID_BRACKET_LEFT, MOD_NONE);
F_Key f_key_backslash(F_BACKSLASH, HID_BACKSLASH, MOD_NONE);
F_Key f_key_bracket_right(F_BRACKET_RIGHT, HID_BRACKET_RIGHT, MOD_NONE);
F_Key f_key_caret(F_CARET, HID_SIX, MOD_SHIFT_L);
F_Key f_key_underscore(F_UNDERSCORE, HID_MINUS, MOD_SHIFT_L);
F_Key f_key_grave(F_GRAVE, HID_GRAVE, MOD_NONE);
F_Key f_key_brace_left(F_BRACE_LEFT, HID_BRACKET_LEFT, MOD_SHIFT_L);
F_Key f_key_pipe(F_PIPE, HID_BACKSLASH, MOD_SHIFT_L);
F_Key f_key_brace_right(F_BRACE_RIGHT, HID_BRACKET_RIGHT, MOD_SHIFT_L);
F_Key f_key_tilde(F_TILDE, HID_GRAVE, MOD_SHIFT_L);
F_Key f_key_capslock(F_CAPSLOCK, HID_CAPLK, MOD_NONE);
F_Key f_key_f1(F_F1, HID_F1, MOD_NONE);
F_Key f_key_f2(F_F2, HID_F2, MOD_NONE);
F_Key f_key_f3(F_F3, HID_F3, MOD_NONE);
F_Key f_key_f4(F_F4, HID_F4, MOD_NONE);
F_Key f_key_f5(F_F5, HID_F5, MOD_NONE);
F_Key f_key_f6(F_F6, HID_F6, MOD_NONE);
F_Key f_key_f7(F_F7, HID_F7, MOD_NONE);
F_Key f_key_f8(F_F8, HID_F8, MOD_NONE);
F_Key f_key_f9(F_F9, HID_F9, MOD_NONE);
F_Key f_key_f10(F_F10, HID_F10, MOD_NONE);
F_Key f_key_f11(F_F11, HID_F11, MOD_NONE);
F_Key f_key_f12(F_F12, HID_F12, MOD_NONE);
F_Key f_key_print_screen(F_PRINT_SCREEN, HID_PRTSC, MOD_NONE);
F_Key f_key_scroll_lock(F_SCROLL_LOCK, HID_SCRLK, MOD_NONE);
F_Key f_key_pause(F_PAUSE, HID_PAUSE, MOD_NONE);
F_Key f_key_insert(F_INSERT, HID_INS, MOD_NONE);
F_Key f_key_home(F_HOME, HID_HOME, MOD_NONE);
F_Key f_key_page_up(F_PAGE_UP, HID_PGUP, MOD_NONE);
F_Key f_key_delete(F_DELETE, HID_DEL, MOD_NONE);
F_Key f_key_end(F_END, HID_END, MOD_NONE);
F_Key f_key_page_down(F_PAGE_DOWN, HID_PGDN, MOD_NONE);
F_Key f_key_arrow_right(F_ARROW_RIGHT, HID_ARW_R, MOD_NONE);
F_Key f_key_arrow_left(F_ARROW_LEFT, HID_ARW_L, MOD_NONE);
F_Key f_key_arrow_down(F_ARROW_DOWN, HID_ARW_D, MOD_NONE);
F_Key f_key_arrow_up(F_ARROW_UP, HID_ARW_U, MOD_NONE);
F_Key f_key_num_lock(F_NUM_LOCK, HID_NUMLK, MOD_NONE);

F_TapAdvanced f_tapbehavior;
F_Key f_kbd_protocol(F_KBDPROTO, HID_NONE, MOD_NONE);
F_KeyboardReset f_mcu_reset(F_MCU_RESET);
F_EventHandler f_chg_oled(F_CHG_OLED);
F_KeyboardReset f_kb_reset(F_KBD_RESET);
F_ToggleKeymapOSLayout f_tgl_oslayout(F_TGL_OSLAYOUT);
F_ChangeProfile f_change_profd(F_CHG_PROFD, PROFILE_DEFAULT);
F_ChangeProfile f_change_prof0(F_CHG_PROF0, PROFILE_0);
F_ChangeProfile f_change_prof1(F_CHG_PROF1, PROFILE_1);
F_ChangeProfile f_change_prof2(F_CHG_PROF2, PROFILE_2);
F_ChangeProfile f_change_prof3(F_CHG_PROF3, PROFILE_3);

F_Mouse f_mouse_btn_left(F_MBTN_LEFT);
F_Mouse f_mouse_btn_right(F_MBTN_RIGHT);
F_Mouse f_mouse_btn_middle(F_MBTN_MIDDLE);
F_Mouse f_mouse_btn_backward(F_MBTN_BW);
F_Mouse f_mouse_btn_forward(F_MBTN_FW);
F_Mouse f_mouse_wheel(F_M_WHEEL);
F_Mouse f_mouse_move(F_MOUSE_MOVE);
F_Touchpad f_touchpad_mouse(F_TOUCHPAD_MOUSE);
F_Touchpad f_touchpad_enable(F_TOUCHPAD_ENABLE);
F_Touchpad f_touchpad_left(F_TOUCHPAD_LEFT);
F_Touchpad f_touchpad_right(F_TOUCHPAD_RIGHT);
F_Touchpad f_touchpad_btn_left(F_TOUCHPAD_BTN_LEFT);
F_Touchpad f_touchpad_btn_right(F_TOUCHPAD_BTN_RIGHT);
F_Touchpad f_touchpad_jiggler(F_JIGGLER);
F_Touchpad f_touchpad_scroll(F_TOUCHPAD_SCROLL);

F_Key f_key_ctrl_left(F_CTRL_LEFT, HID_NONE, MOD_CTR_L);
F_Key f_key_shift_left(F_SHIFT_LEFT, HID_NONE, MOD_SHIFT_L);
F_Key f_key_alt_left(F_ALT_LEFT, HID_NONE, MOD_ALT_L);
F_Key f_key_gui_left(F_GUI_LEFT, HID_NONE, MOD_GUI_L);
F_Key f_key_ctrl_right(F_CTRL_RIGHT, HID_NONE, MOD_CTRL_R);
F_Key f_key_shift_right(F_SHIFT_RIGHT, HID_NONE, MOD_SHIFT_R);
F_Key f_key_alt_right(F_ALT_RIGHT, HID_NONE, MOD_ALT_R);
F_Key f_key_gui_right(F_GUI_RIGHT, HID_NONE, MOD_GUI_R);

F_KeyMacro f_macro00(F_MACRO00, Config::getActiveMacro(0));
F_KeyMacro f_macro01(F_MACRO01, Config::getActiveMacro(1));
F_KeyMacro f_macro02(F_MACRO02, Config::getActiveMacro(2));
F_KeyMacro f_macro03(F_MACRO03, Config::getActiveMacro(3));
F_KeyMacro f_macro04(F_MACRO04, Config::getActiveMacro(4));
F_KeyMacro f_macro05(F_MACRO05, Config::getActiveMacro(5));
//F_KeyMacro f_macro06(F_MACRO06, Config::getActiveMacro(6));
F_Key f_copilot(F_COPILOT, HID_F23, MOD_SHIFT_L|MOD_GUI_L);
//F_KeyMacro *ckm_macros[] = { &f_macro00, &f_macro01, &f_macro02, &f_macro03, &f_macro04, &f_macro05, &f_macro06 };
F_KeyMacro *ckm_macros[] = { &f_macro00, &f_macro01, &f_macro02, &f_macro03, &f_macro04, &f_macro05 };
static_assert(sizeof(ckm_macros)/sizeof(ckm_macros[0]) == DEFS_NMACROS,
  "ckm_macros[] != DEFS_NMACROS. Check initialization list or DEFS_NMACROS value.");

F_SelectLayer f_layer_calibration(F_LAYER_CALIBRATION, LAYER_CALIBRATION);
F_SelectLayer f_exit_emergency(F_EXIT_EMERGENCY, LAYER_0);
F_SelectLayer f_layer_1(F_LAYER_1, LAYER_1);
F_SelectLayer f_layer_2(F_LAYER_2, LAYER_2);
F_SelectLayer f_layer_3(F_LAYER_3, LAYER_3);
F_SelectLayer f_layer_hold(F_LAYER_HOLD, LAYER_0);

F_Key f_key_dquote_jp(F_DQUOTE, HID_TWO, MOD_SHIFT_L);
F_Key f_key_ampersand_jp(F_AMPERSAND, HID_SIX, MOD_SHIFT_L);
F_Key f_key_quote_jp(F_QUOTE, HID_SEVEN, MOD_SHIFT_L);
F_Key f_key_paren_left_jp(F_PAREN_LEFT, HID_EIGHT, MOD_SHIFT_L);
F_Key f_key_paren_right_jp(F_PAREN_RIGHT, HID_NINE, MOD_SHIFT_L);
F_Key f_key_asterisk_jp(F_ASTERISK, HID_QUOTE, MOD_SHIFT_L);
F_Key f_key_plus_jp(F_PLUS, HID_SEMICOLON, MOD_SHIFT_L);
F_Key f_key_colon_jp(F_COLON, HID_QUOTE, MOD_NONE);
F_Key f_key_equal_jp(F_EQUAL, HID_MINUS, MOD_SHIFT_L);
F_Key f_key_at_jp(F_AT, HID_BRACKET_LEFT, MOD_NONE);
F_Key f_key_bracket_left_jp(F_BRACKET_LEFT, HID_BRACKET_RIGHT, MOD_NONE);
F_Key f_key_backslash_jp(F_BACKSLASH, HID_JP_BACKSLASH, MOD_NONE);
F_Key f_key_bracket_right_jp(F_BRACKET_RIGHT, HID_JP_BRACKET_RIGHT, MOD_NONE);
F_Key f_key_caret_jp(F_CARET, HID_EQUAL, MOD_NONE);
F_Key f_key_underscore_jp(F_UNDERSCORE, HID_JP_BACKSLASH, MOD_SHIFT_L);
F_Key f_key_grave_jp(F_GRAVE, HID_BRACKET_LEFT, MOD_SHIFT_L);
F_Key f_key_brace_left_jp(F_BRACE_LEFT, HID_BRACKET_RIGHT, MOD_SHIFT_L);
F_Key f_key_pipe_jp(F_PIPE, HID_JP_YEN, MOD_SHIFT_L);
F_Key f_key_brace_right_jp(F_BRACE_RIGHT, HID_JP_BRACKET_RIGHT, MOD_SHIFT_L);
F_Key f_key_tilde_jp(F_TILDE, HID_EQUAL, MOD_SHIFT_L);
F_Key f_key_hanzen_jp(F_HANZEN, HID_GRAVE, MOD_NONE);
F_Key f_key_hirakana_jp(F_HIRAKANA, HID_JP_KANA, MOD_NONE);
F_Key f_key_yen_jp(F_YEN, HID_JP_YEN, MOD_NONE);
F_Key f_key_convert_jp(F_CONVERT, HID_JP_CONV, MOD_NONE);
F_Key f_key_noconvert_jp(F_NOCONVERT, HID_JP_NOCONV, MOD_NONE);

FunctionInstance CustomKeymap::f_instances_setting[256] = {
  { F_BS, &f_key_bs, &f_key_bs, "Backspace", "BS" },
  { F_TAB, &f_key_tab, &f_key_tab, "Tab", "Tab" },
  { F_ENTER, &f_key_enter, &f_key_enter, "Enter", "Ent" },
  { F_ESC, &f_key_esc, &f_key_esc, "Escape", "Esc" },
  { F_SPACE, &f_key_space, &f_key_space, "Space", "Spc" },
  { F_EXCL, &f_key_excl, &f_key_excl, "Exclamation", "!" },
  { F_DQUOTE, &f_key_dquote, &f_key_dquote_jp, "Double Quote", "\"" },
  { F_HASH, &f_key_hash, &f_key_hash, "Hash", "#" },
  { F_DOLLAR, &f_key_dollar, &f_key_dollar, "Dollar", "$" },
  { F_PERCENT, &f_key_percent, &f_key_percent, "Percent", "%" },
  { F_AMPERSAND, &f_key_ampersand, &f_key_ampersand_jp, "Ampersand", "&" },
  { F_QUOTE, &f_key_quote, &f_key_quote_jp, "Quote", "'" },
  { F_PAREN_LEFT, &f_key_paren_left, &f_key_paren_left_jp, "Left Parenthesis", "(" },
  { F_PAREN_RIGHT, &f_key_paren_right, &f_key_paren_right_jp, "Right Parenthesis", ")" },
  { F_ASTERISK, &f_key_asterisk, &f_key_asterisk_jp, "Asterisk", "*" },
  { F_PLUS, &f_key_plus, &f_key_plus_jp, "Plus", "+" },
  { F_COMMA, &f_key_comma, &f_key_comma, "Comma", "," },
  { F_MINUS, &f_key_minus, &f_key_minus, "Minus", "-" },
  { F_PERIOD, &f_key_period, &f_key_period, "Period", "." },
  { F_SLASH, &f_key_slash, &f_key_slash, "Slash", "/" },
  { F_0, &f_key_0, &f_key_0, "0", "0" },
  { F_1, &f_key_1, &f_key_1, "1", "1" },
  { F_2, &f_key_2, &f_key_2, "2", "2" },
  { F_3, &f_key_3, &f_key_3, "3", "3" },
  { F_4, &f_key_4, &f_key_4, "4", "4" },
  { F_5, &f_key_5, &f_key_5, "5", "5" },
  { F_6, &f_key_6, &f_key_6, "6", "6" },
  { F_7, &f_key_7, &f_key_7, "7", "7" },
  { F_8, &f_key_8, &f_key_8, "8", "8" },
  { F_9, &f_key_9, &f_key_9, "9", "9" },
  { F_COLON, &f_key_colon, &f_key_colon_jp, "Colon", ":" },
  { F_SEMICOLON, &f_key_semicolon, &f_key_semicolon, "Semicolon", ";" },
  { F_ANGLE_LEFT, &f_key_angle_left, &f_key_angle_left, "Left Angle", "<" },
  { F_EQUAL, &f_key_equal, &f_key_equal_jp, "Equal", "=" },
  { F_ANGLE_RIGHT, &f_key_angle_right, &f_key_angle_right, "Right Angle", ">" },
  { F_QUESTION, &f_key_question, &f_key_question, "Question", "?" },
  { F_AT, &f_key_at, &f_key_at_jp, "At", "@" },
  { F_A, &f_key_A, &f_key_A, "A", "A" },
  { F_B, &f_key_B, &f_key_B, "B", "B" },
  { F_C, &f_key_C, &f_key_C, "C", "C" },
  { F_D, &f_key_D, &f_key_D, "D", "D" },
  { F_E, &f_key_E, &f_key_E, "E", "E" },
  { F_F, &f_key_F, &f_key_F, "F", "F" },
  { F_G, &f_key_G, &f_key_G, "G", "G" },
  { F_H, &f_key_H, &f_key_H, "H", "H" },
  { F_I, &f_key_I, &f_key_I, "I", "I" },
  { F_J, &f_key_J, &f_key_J, "J", "J" },
  { F_K, &f_key_K, &f_key_K, "K", "K" },
  { F_L, &f_key_L, &f_key_L, "L", "L" },
  { F_M, &f_key_M, &f_key_M, "M", "M" },
  { F_N, &f_key_N, &f_key_N, "N", "N" },
  { F_O, &f_key_O, &f_key_O, "O", "O" },
  { F_P, &f_key_P, &f_key_P, "P", "P" },
  { F_Q, &f_key_Q, &f_key_Q, "Q", "Q" },
  { F_R, &f_key_R, &f_key_R, "R", "R" },
  { F_S, &f_key_S, &f_key_S, "S", "S" },
  { F_T, &f_key_T, &f_key_T, "T", "T" },
  { F_U, &f_key_U, &f_key_U, "U", "U" },
  { F_V, &f_key_V, &f_key_V, "V", "V" },
  { F_W, &f_key_W, &f_key_W, "W", "W" },
  { F_X, &f_key_X, &f_key_X, "X", "X" },
  { F_Y, &f_key_Y, &f_key_Y, "Y", "Y" },
  { F_Z, &f_key_Z, &f_key_Z, "Z", "Z" },
  { F_BRACKET_LEFT, &f_key_bracket_left, &f_key_bracket_left_jp, "Left Bracket", "[" },
  { F_BACKSLASH, &f_key_backslash, &f_key_backslash_jp, "Backslash", "\\" },
  { F_BRACKET_RIGHT, &f_key_bracket_right, &f_key_bracket_right_jp, "Right Bracket", "]" },
  { F_CARET, &f_key_caret, &f_key_caret_jp, "Caret", "^" },
  { F_UNDERSCORE, &f_key_underscore, &f_key_underscore_jp, "Underscore", "_" },
  { F_GRAVE, &f_key_grave, &f_key_grave_jp, "Grave", "`" },
  { F_a, &f_key_a, &f_key_a, "a", "a" },
  { F_b, &f_key_b, &f_key_b, "b", "b" },
  { F_c, &f_key_c, &f_key_c, "c", "c" },
  { F_d, &f_key_d, &f_key_d, "d", "d" },
  { F_e, &f_key_e, &f_key_e, "e", "e" },
  { F_f, &f_key_f, &f_key_f, "f", "f" },
  { F_g, &f_key_g, &f_key_g, "g", "g" },
  { F_h, &f_key_h, &f_key_h, "h", "h" },
  { F_i, &f_key_i, &f_key_i, "i", "i" },
  { F_j, &f_key_j, &f_key_j, "j", "j" },
  { F_k, &f_key_k, &f_key_k, "k", "k" },
  { F_l, &f_key_l, &f_key_l, "l", "l" },
  { F_m, &f_key_m, &f_key_m, "m", "m" },
  { F_n, &f_key_n, &f_key_n, "n", "n" },
  { F_o, &f_key_o, &f_key_o, "o", "o" },
  { F_p, &f_key_p, &f_key_p, "p", "p" },
  { F_q, &f_key_q, &f_key_q, "q", "q" },
  { F_r, &f_key_r, &f_key_r, "r", "r" },
  { F_s, &f_key_s, &f_key_s, "s", "s" },
  { F_t, &f_key_t, &f_key_t, "t", "t" },
  { F_u, &f_key_u, &f_key_u, "u", "u" },
  { F_v, &f_key_v, &f_key_v, "v", "v" },
  { F_w, &f_key_w, &f_key_w, "w", "w" },
  { F_x, &f_key_x, &f_key_x, "x", "x" },
  { F_y, &f_key_y, &f_key_y, "y", "y" },
  { F_z, &f_key_z, &f_key_z, "z", "z" },
  { F_BRACE_LEFT, &f_key_brace_left, &f_key_brace_left_jp, "Left Brace", "{" },
  { F_PIPE, &f_key_pipe, &f_key_pipe_jp, "Pipe", "|" },
  { F_BRACE_RIGHT, &f_key_brace_right, &f_key_brace_right_jp, "Right Brace", "}" },
  { F_TILDE, &f_key_tilde, &f_key_tilde_jp, "Tilde", "~" },

  { F_CAPSLOCK, &f_key_capslock, &f_key_capslock, "Caps Lock", "CapsL" },
  { F_F1, &f_key_f1, &f_key_f1, "F1", "F1" },
  { F_F2, &f_key_f2, &f_key_f2, "F2", "F2" },
  { F_F3, &f_key_f3, &f_key_f3, "F3", "F3" },
  { F_F4, &f_key_f4, &f_key_f4, "F4", "F4" },
  { F_F5, &f_key_f5, &f_key_f5, "F5", "F5" },
  { F_F6, &f_key_f6, &f_key_f6, "F6", "F6" },
  { F_F7, &f_key_f7, &f_key_f7, "F7", "F7" },
  { F_F8, &f_key_f8, &f_key_f8, "F8", "F8" },
  { F_F9, &f_key_f9, &f_key_f9, "F9", "F9" },
  { F_F10, &f_key_f10, &f_key_f10, "F10", "F10" },
  { F_F11, &f_key_f11, &f_key_f11, "F11", "F11" },
  { F_F12, &f_key_f12, &f_key_f12, "F12", "F12" },
  { F_PRINT_SCREEN, &f_key_print_screen, &f_key_print_screen, "Print Screen", "PrtSc" },
  { F_SCROLL_LOCK, &f_key_scroll_lock, &f_key_scroll_lock, "Scroll Lock", "ScrL" },
  { F_PAUSE, &f_key_pause, &f_key_pause, "Pause", "Pau" },
  { F_INSERT, &f_key_insert, &f_key_insert, "Insert", "Ins" },
  { F_HOME, &f_key_home, &f_key_home, "Home", "Home" },
  { F_PAGE_UP, &f_key_page_up, &f_key_page_up, "Page Up", "PgUP" },
  { F_DELETE, &f_key_delete, &f_key_delete, "Delete", "Del" },
  { F_END, &f_key_end, &f_key_end, "End", "End" },
  { F_PAGE_DOWN, &f_key_page_down, &f_key_page_down, "Page Down", "PgDN" },
  { F_ARROW_RIGHT, &f_key_arrow_right, &f_key_arrow_right, "Arrow Right", "ArwR" },
  { F_ARROW_LEFT, &f_key_arrow_left, &f_key_arrow_left, "Arrow Left", "ArwL" },
  { F_ARROW_DOWN, &f_key_arrow_down, &f_key_arrow_down, "Arrow Down", "ArwD" },
  { F_ARROW_UP, &f_key_arrow_up, &f_key_arrow_up, "Arrow Up", "ArwU" },
  { F_NUM_LOCK, &f_key_num_lock, &f_key_num_lock, "Num Lock", "NumL" },
  { F_HANZEN, &f_none, &f_key_hanzen_jp, "None", nullptr },
  { F_HIRAKANA, &f_none, &f_key_hirakana_jp, "None", nullptr },
  { F_YEN, &f_none, &f_key_yen_jp, "None", nullptr },
  { F_CONVERT, &f_none, &f_key_convert_jp, "None", nullptr },
  { F_NOCONVERT, &f_none, &f_key_noconvert_jp, "None", nullptr },

  // FUNCTIONS: 0xB0 <= code < 0xD0
  { F_TAPADVANCED, &f_tapbehavior, &f_tapbehavior, "Toggle Tap Behavior", "TapB" },
  { F_KBDPROTO, &f_kbd_protocol, &f_kbd_protocol, "Keyboard Protocol", "KBDP" },
#if defined(SUPPORT_TOUCHPAD)
  { F_JIGGLER, &f_touchpad_jiggler, &f_touchpad_jiggler, "Jiggler", "JIGG" },
#endif
#if defined(SUPPORT_MOUSE)
  { F_JIGGLER, &ctxreporter.f_mouse_reporter, &ctxreporter.f_mouse_reporter, "Jiggler", "JIGG" },
#endif
  { F_BRIGHTNESS, &ctxsys.f_systemled, &ctxsys.f_systemled, "LED AUTO", "LEDA" },
  { F_BRIGHTDOWN, &ctxsys.f_systemled, &ctxsys.f_systemled, "LED Brightness Down", "LED-" },
  { F_BRIGHTUP, &ctxsys.f_systemled, &ctxsys.f_systemled, "LED Brightness Up", "LED+" },
  { F_KLED_BACKLIGHT, &ctxsys.f_systemled, &ctxsys.f_systemled, "KeyLED Backlight", "LEDB" },
  { F_MCU_RESET, &f_mcu_reset, &f_mcu_reset, "MCU Reset", "M_RST" },
  { F_CHG_OLED, &f_chg_oled, &f_chg_oled, "Change OLED", "OLED" },
  { F_KBD_RESET, &f_kb_reset, &f_kb_reset, "Keyboard Reset", "RST" },
  { F_TGL_OSLAYOUT, &f_tgl_oslayout, &f_tgl_oslayout, "Toggle OS Layout", "TGL" },
  { F_CHG_PROFD, &f_change_profd, &f_change_profd, "Change Profile Def", "PROFD" },
  { F_CHG_PROF0, &f_change_prof0, &f_change_prof0, "Change Profile 0", "PROF0" },
  { F_CHG_PROF1, &f_change_prof1, &f_change_prof1, "Change Profile 1", "PROF1" },
  { F_CHG_PROF2, &f_change_prof2, &f_change_prof2, "Change Profile 2", "PROF2" },
  { F_CHG_PROF3, &f_change_prof3, &f_change_prof3, "Change Profile 3", "PROF3" },

  // MOUSE: code & 0xF0 = 0xD0
  { F_MBTN_LEFT, &f_mouse_btn_left, &f_mouse_btn_left, "Mouse Button Left", "MBL" },
  { F_MBTN_RIGHT, &f_mouse_btn_right, &f_mouse_btn_right, "Mouse Button Right", "MBR" },
  { F_MBTN_MIDDLE, &f_mouse_btn_middle, &f_mouse_btn_middle, "Mouse Button Middle", "MBM" },
  { F_MBTN_BW, &f_mouse_btn_backward, &f_mouse_btn_backward, "Mouse Button Backward", "MBB" },
  { F_MBTN_FW, &f_mouse_btn_forward, &f_mouse_btn_forward, "Mouse Button Forward", "MFB" },
  { F_M_WHEEL, &f_mouse_wheel, &f_mouse_wheel, "Mouse Wheel", "MWHL" },
  { F_MOUSE_MOVE, &f_mouse_move, &f_mouse_move, "Mouse Move", "MMOVE" },

  { F_TOUCHPAD_SCROLL, &f_touchpad_scroll, &f_touchpad_scroll, "Touchpad Scroll", "TPSc" },
  { F_TOUCHPAD_MOUSE, &f_touchpad_mouse, &f_touchpad_mouse, "Touchpad Mouse mode", "TPMou" },
  { F_TOUCHPAD_ENABLE, &f_touchpad_enable, &f_touchpad_enable, "Touchpad Enable", "TPEn" },
  { F_TOUCHPAD_LEFT, &f_touchpad_left, &f_touchpad_left, "Touchpad Left", "TPL" },
  { F_TOUCHPAD_LEFT2X, &f_touchpad_left, &f_touchpad_left, "Touchpad Left 2X", "TPL2X" },
  { F_TOUCHPAD_RIGHT, &f_touchpad_right, &f_touchpad_right, "Touchpad Right", "TPR" },
  { F_TOUCHPAD_RIGHT2X, &f_touchpad_right, &f_touchpad_right, "Touchpad Right 2X", "TPR2X" },
  { F_TOUCHPAD_BTN_LEFT, &f_touchpad_btn_left, &f_touchpad_btn_left, "TP Button Left", "TPBL" },
  { F_TOUCHPAD_BTN_RIGHT, &f_touchpad_btn_right, &f_touchpad_btn_right, "TP Button Right", "TPBR" },

  // MODIFIER: code & 0xF8 = 0xE0
  { F_CTRL_LEFT, &f_key_ctrl_left, &f_key_ctrl_left, "Control Left", "CtrL" },
  { F_SHIFT_LEFT, &f_key_shift_left, &f_key_shift_left, "Shift Left", "ShfL" },
  { F_ALT_LEFT, &f_key_alt_left, &f_key_alt_left, "Alt Left", "AltL" },
  { F_GUI_LEFT, &f_key_gui_left, &f_key_gui_left, "GUI Left", "GuiL" },
  { F_CTRL_RIGHT, &f_key_ctrl_right, &f_key_ctrl_right, "Control Right", "CtrR" },
  { F_SHIFT_RIGHT, &f_key_shift_right, &f_key_shift_right, "Shift Right", "ShfR" },
  { F_ALT_RIGHT, &f_key_alt_right, &f_key_alt_right, "Alt Right", "AltR" },
  { F_GUI_RIGHT, &f_key_gui_right, &f_key_gui_right, "GUI Right", "GuiR" },
  { F_MACRO00, &f_macro00, &f_macro00, "Macro 00", "MCR00" },
  { F_MACRO01, &f_macro01, &f_macro01, "Macro 01", "MCR01" },
  { F_MACRO02, &f_macro02, &f_macro02, "Macro 02", "MCR02" },
  { F_MACRO03, &f_macro03, &f_macro03, "Macro 03", "MCR03" },
  { F_MACRO04, &f_macro04, &f_macro04, "Macro 04", "MCR04" },
  { F_MACRO05, &f_macro05, &f_macro05, "Macro 05", "MCR05" },
  { F_COPILOT, &f_copilot, &f_copilot, "Copilot", "COPLT" },

  // LAYER: code & 0xF0 = 0xF0
  { F_LAYER_CALIBRATION, &f_layer_calibration, &f_layer_calibration, "Layer Calibration", "CAL" },
  { F_LAYER_1, &f_layer_1, &f_layer_1, "Layer 1", "L1" },
  { F_LAYER_2, &f_layer_2, &f_layer_2, "Layer 2", "L2" },
  { F_LAYER_3, &f_layer_3, &f_layer_3, "Layer 3", "L3" },
  { F_LAYER_HOLD, &f_layer_hold, &f_layer_hold, "Layer Hold", "LHold" },
  { F_EXIT_EMERGENCY, &f_exit_emergency, &f_exit_emergency, "Exit Emergency", "ExEm" },
};

FunctionCode_t CustomKeymap::keylist[14*13] = {
  F_TAB, F_q, F_w, F_e, F_r, F_t, F_y, F_u, F_i, F_o, F_p, F_BRACKET_LEFT, F_BRACKET_RIGHT, F_BS,
  F_ESC, F_Q, F_W, F_E, F_R, F_T, F_Y, F_U, F_I, F_O, F_P, F_BRACE_LEFT, F_BRACE_RIGHT, F_DELETE,
  F_CTRL_LEFT, F_a, F_s, F_d, F_f, F_g, F_h, F_j, F_k, F_l, F_SEMICOLON, F_QUOTE, F_NONE, F_ENTER,
  F_NONE, F_A, F_S, F_D, F_F, F_G, F_H, F_J, F_K, F_L, F_COLON, F_DQUOTE, F_NONE, F_NONE,
  F_SHIFT_LEFT, F_z, F_x, F_c, F_v, F_b, F_n, F_m, F_COMMA, F_PERIOD, F_SLASH, F_BACKSLASH, F_SHIFT_RIGHT,
  F_NONE, F_Z, F_X, F_C, F_V, F_B, F_N, F_M, F_ANGLE_LEFT, F_ANGLE_RIGHT, F_QUESTION, F_PIPE, F_NONE,
  F_GRAVE, F_1, F_2, F_3, F_4, F_5, F_6, F_7, F_8, F_9, F_0, F_MINUS, F_EQUAL, F_NONE,
  F_TILDE, F_EXCL, F_AT, F_HASH, F_DOLLAR, F_PERCENT, F_CARET, F_AMPERSAND, F_ASTERISK, F_PAREN_LEFT, F_PAREN_RIGHT, F_UNDERSCORE, F_PLUS, F_NONE,
  F_NONE, F_GUI_LEFT, F_ALT_LEFT, F_NOCONVERT, F_SPACE, F_CONVERT, F_ALT_RIGHT, F_GUI_RIGHT, F_CTRL_RIGHT, F_ARROW_LEFT, F_ARROW_DOWN, F_ARROW_RIGHT, F_ARROW_UP, F_NONE,
  F_NONE, F_F1, F_F2, F_F3, F_F4, F_F5, F_F6, F_F7, F_F8, F_F9, F_F10, F_F11, F_F12, F_NONE,
  F_CHG_OLED, F_KBD_RESET, F_TGL_OSLAYOUT, F_CHG_PROFD, F_CHG_PROF0, F_CHG_PROF1, F_CHG_PROF2, F_CHG_PROF3, F_MBTN_LEFT, F_MBTN_RIGHT, F_MBTN_MIDDLE, F_MBTN_BW, F_MBTN_FW, F_M_WHEEL,
  F_LAYER_CALIBRATION, F_LAYER_1, F_LAYER_2, F_LAYER_3, F_MACRO00, F_MACRO01, F_MACRO02, F_MACRO03, F_MACRO04, F_MACRO05, F_MACRO06, F_COPILOT, F_NONE, F_NONE,
  F_PAUSE, F_INSERT, F_HOME, F_END, F_PAGE_UP, F_PAGE_DOWN, F_HANZEN, F_HIRAKANA, F_YEN, F_NONE, F_PRINT_SCREEN, F_NONE, F_NONE, F_NONE,
};

bool CustomKeymap::setupFunctionInstances() {
  bool r = true;

  for(int i = 0; i < sizeof(f_instances)/sizeof(f_instances[0]); i++) {
    f_instances[i] = { F_NONE, &f_none, &f_none, "None", nullptr };
  }

  for(int i = 0; i < sizeof(f_instances_setting)/sizeof(f_instances_setting[0]); i++) {
    FunctionCode_t fcode = f_instances_setting[i].fcode;

    if(fcode == F_NONE) {
      continue;
    } else if(f_instances[fcode].fcode != F_NONE) {
      LOG_ERROR("f_instances_setting[] duplicate, code", fcode);
      r = false;
    } else {
      f_instances[fcode] = f_instances_setting[i];
    }
  }

  return r;
}

bool CustomKeymap::fetchFunctionKeylist(uint16_t index, FunctionCode_t &fcode, const char *&disp) {
  if(index >= sizeof(keylist)/sizeof(keylist[0])) return false;
  fcode = keylist[index];
  disp = f_instances[fcode].disp;
  return true;
}

bool CustomKeymap::setOSLayout(KeymapOSLayoutType layout) {
  bool r = false;
  if(validateOSLayout(layout)) {
    keymap.layout = layout;
    r = true;
  }
  return r;
}

/*
 * for keymap.ftable[] and keymap.funit[]
*/
bool CustomKeymap::setFunctionCode(uint8_t key, KeyswState state, KeymapLayerType layer, FunctionCode_t keycode) {
  if(!validateFunctionCode(key, state, layer, keycode)) return false;

  if(key < DEFS_NMAGKEYS && state == KS_PRESS && layer < KEYMAPTABLE_MAX) {
    keymap.ftable[layer][key] = keycode;

  } else {
    uint8_t i = 0;
    // overwrite if duplication
    while(i < num_funit) {
      if(keymap.funit[i].key == key && keymap.funit[i].state == state && keymap.funit[i].layer == layer) {
        if(keycode == F_NONE) {
          keymap.funit[i] = empty;
          alignFunctionUnit();
        } else {
          keymap.funit[i].func = keycode;
        }
        return true;
      }
      i++;
    }

    if(i >= sizeof(keymap.funit)/sizeof(keymap.funit[0])) {
      LOG_ERROR("keymap.funit is FULL");
      return false;
    }

    // if(i < num_funit) overwrite existing entry
    // if(i == num_funit) new entry
    keymap.funit[i].key = key;
    keymap.funit[i].state = state;
    keymap.funit[i].layer = layer;
    keymap.funit[i].func = keycode;
    num_funit++;
    alignFunctionUnit(); // needed in the case the value is overwritten to F_NONE.
  }

  return true;
}

FunctionCode_t CustomKeymap::searchInTables(uint8_t key, KeyswState state, KeymapLayerType layer) {
  if(key < DEFS_NMAGKEYS && state == KS_PRESS && layer < KEYMAPTABLE_MAX) {
    return(keymap.ftable[layer][key]);
  }
  return F_NONE;
}

FunctionCode_t CustomKeymap::searchInUnits(uint8_t key, KeyswState state, KeymapLayerType layer) {
  uint8_t i = 0;
  while(i < num_funit) {
    if(keymap.funit[i].key == key && keymap.funit[i].state == state && keymap.funit[i].layer == layer) {
      return keymap.funit[i].func;
    }
    i++;
  }

  i = 0; 
  while(i < num_funit_common) {
    if(funit_common[i].key == key && funit_common[i].state == state && funit_common[i].layer == layer) {
      return funit_common[i].func;
    }
    i++;
  }

  return F_NONE;
}

FunctionCode_t CustomKeymap::getFunctionCode(uint8_t key, KeyswState state, KeymapLayerType layer) {
  if(!validateKey(key) || !validateLayer(layer) || state >= KS_MAX) return F_NONE;

  FunctionCode_t f = F_NONE;

  switch(state) {
    case KS_HOLDLL:
      f = searchInUnits(key, state, layer);
      if(f != F_NONE) break;

      case KS_HOLDL:
      f = searchInUnits(key, KS_HOLDL ,layer);
      if(f != F_NONE) break;

    case KS_HOLD:
      f = searchInUnits(key, KS_HOLD, layer);
      if(f != F_NONE) break;

    case KS_PRESS:
      f = searchInTables(key, KS_PRESS, layer);
      if(f != F_NONE) break;

    case KS_PARTIAL:
    case KS_TAP:
      f = searchInUnits(key, state, layer);
      break;

    case KS_VALUE:
      f = searchInUnits(key, state, layer);
      break;

    default:
      LOG_ERROR("invalid keystate", state);
      break;
  }

  return f;
}


bool CustomKeymap::alignFunctionUnit() {
  Keymap &km = keymap;
  uint8_t nfunit = 0;

  // check duplication
  for(uint8_t i = 0; i < sizeof(km.funit)/sizeof(km.funit[0]); i++) {
    FunctionUnit &ki = km.funit[i];
    if(ki.state == KS_RELEASE) continue;

    uint8_t j = i + 1;
    while(j < sizeof(km.funit)/sizeof(km.funit[0])) {
      FunctionUnit &ki2 = km.funit[j++];
      if(ki.key == ki2.key && ki.state == ki2.state && ki.layer == ki2.layer) ki2 = empty;
    }
  }

  // fill in the front
  nfunit = 0;
  for(uint8_t i = 0; i < sizeof(km.funit)/sizeof(km.funit[0]); i++) {
    if(km.funit[i].state == KS_RELEASE) {
      uint8_t j = i + 1;
      while(j < sizeof(km.funit)/sizeof(km.funit[0])) {
        if(km.funit[j].state != KS_RELEASE) {
          km.funit[i] = km.funit[j];
          km.funit[j] = empty;
          nfunit++;
          break;
        }
        j++;
      }
    } else nfunit++;
  }

  num_funit = nfunit;
  return true;
}

bool CustomKeymap::validateFunctionCode(uint8_t key, KeyswState state, KeymapLayerType layer, FunctionCode_t keycode) {
  // basic parameter check
  // layout is not irrelevant.
  if(!validateKey(key) || !validateLayer(layer) || state >= KS_MAX) {
    LOG_ERROR("invalid range of some value. key", key, ", layer", layer, ", state", state);
    return false;
  }

  // RELEASE not allowed.
  // KS_PRESS allowed in every layer.
  if(state == KS_RELEASE) {
    LOG_ERROR("keystate RELEASE not allowed in keymap");
    return false;
  }

  // check duplicate definition in funit_common
  uint8_t i = 0;
  while(i < num_funit_common) {
    if(funit_common[i].key == key && funit_common[i].state == state && funit_common[i].layer == layer) {
      LOG_ERROR("duplicate definition in funit_common. key", key, ", layer", layer, ", state", state);
      return false;
    }
    i++;
  }

  return true;
}

/*
 * validate the keymap.
 * change them if invalid.
 */
bool CustomKeymap::validateKeymap(Keymap &km) {
  bool r = true;

  if(!validateOSLayout(km.layout)) {
    LOG_ERROR("Invalid layout ", km.layout);
    km.layout = OSLAYOUT_US;
    r = false;
  }

  // MagKeys
  for(uint8_t key = 0; key < DEFS_NMAGKEYS; key++) {
    for(uint8_t layer = 0; layer < KEYMAPTABLE_MAX; layer++) {
      if(!validateFunctionCode(key, KS_PRESS, (KeymapLayerType)layer, km.ftable[layer][key])) {
        LOG_ERROR("Invalid code, key ", key, ", layer ", layer,
          ", fcode ", DebugLogBase::HEX, km.ftable[layer][key]);
        km.ftable[layer][key] = F_NONE;
        r = false;
      }
    }
  }

  // Other inputs
  // if the key and layer are defined in funit_common, skip it.
  for(uint8_t i = 0; i < sizeof(km.funit) / sizeof(km.funit[0]); i++) {
    FunctionUnit &ki = km.funit[i];
    if(!ki.state) continue; // empty

    if(!validateFunctionCode(ki.key, ki.state, ki.layer, ki.func)) {
      LOG_ERROR("Invalid custom key ", ki.key, ", state ", ki.state,
        ", layer ", ki.layer, ", keycode ", DebugLogBase::HEX, ki.func);
      ki = empty;
      r = false;
    }
  }

  return r;
}


/* 
  f_layer_handler instance runs continuously in the main loop.

  tact_sw longhold1
  -> enter LAYER_CALIBRATION

  tact_sw press /w LAYER_CALIBRATION
  -> exit LAYER_CALIBRATION
  */
ActionResult F_SelectLayer::childaction(unsigned long ms, FunctionCode_t fc) {
  switch(fcode) {
    case F_NONE: // F_PR_LOW
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode() && set_holded_layer) {
        LOG_DEBUG("set", set_holded_layer, ", holding", holding, ", holded", holded_layer, ", selected", selected_layer);
      }
#endif
      if(set_holded_layer) {
        holded_layer = selected_layer;
        set_holded_layer = false;
      }
      if(holding) { // use holded_layer while holding
        selected_layer = holded_layer;
      }
      break;

    case F_LAYER_CALIBRATION:
      if(!beginning) {
        return F_RES_NONE; // debounce
      }
      if(running) { //exit CALIBRATION
#if defined(DEBUG)
        if(ctxsys.kb_ctrl.isDebugMode()) {
          LOG_DEBUG("Exit Calibration mode");
        }
#endif
        running = false;
        holding = false;
        set_holded_layer = true;
        exitCalibration();
        return F_RES_SUCCESS; // do not update selected_layer

      } else { // enter CALIBRATION
#if defined(DEBUG)
        if(ctxsys.kb_ctrl.isDebugMode()) {
          LOG_DEBUG("Enter Calibration mode");
        }
#endif
        running = true;
        holding = true;
        set_holded_layer = true;
        enterCalibration();
      } // continue to set selected_layer = LAYER_CALIBRATION

    case F_LAYER_1:
    case F_LAYER_2:
    case F_LAYER_3:
      if(selected_layer < target_layer) {
        selected_layer = target_layer;
      }
      break;

    case F_LAYER_HOLD:
      if(!beginning) {
        return F_RES_NONE; // debounce
      }
      holding = !holding;
      if(holding) set_holded_layer = true;
      break;

    case F_EXIT_EMERGENCY:
      if(!beginning) {
        return F_RES_NONE; // debounce
      }
      ctxsys.kb_ctrl.enableEmergencyMode(false);
      ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "Exit", "Emergency Mode", SystemEventQ::DUP_OVERWRITE);
      break;

    default:
      LOG_ERROR("unknown layer function:", DebugLogBase::HEX, fcode);
      return F_RES_FAIL;
  }

  return F_RES_SUCCESS;
}

uint8_t F_SelectLayer::enterCalibration() {
  ctxsys.kb_ctrl.enableCalibrationMode(true);
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_CALIBRATION, -1, "Enter", "Calibration Mode");
  return(KeySensorMCU::calibrationMode(true));
}
uint8_t F_SelectLayer::exitCalibration() {
  ctxsys.kb_ctrl.enableCalibrationMode(false);
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_CALIBRATION, 1, "Exit", "Calibration Mode", SystemEventQ::DUP_OVERWRITE);
  return(KeySensorMCU::calibrationMode(false));
}

ActionResult F_ToggleKeymapOSLayout::childaction(unsigned long ms, FunctionCode_t fc) {
  if(!beginning) {
    return F_RES_NONE; // debounce
  }

  uint8_t layout = (uint8_t)ctxkeymap.keymap.getOSLayout();
  layout++;
  if(layout >= (uint8_t)OSLAYOUT_MAX) layout = 0;
  ctxkeymap.keymap.setOSLayout((KeymapOSLayoutType)layout);
  ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1, "OS Layout", ctxkeymap.keymap.getOSLayoutStr());
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("OS Layout changed to", layout);
  }
#endif
  return F_RES_SUCCESS;
}


const char *CustomKeymap::macroCmd = "macro";
const char *CustomKeymap::macroSyntax = "macro 0-6(macro) [duration simul fcode[ fcode...]]";
const char *CustomKeymap::macroDescr = "Set key macros. or reset macro";
int CustomKeymap::macro(Stream *s, uint8_t ac, const char **av) {
  if(ac < 1) return -1;
  uint8_t mindex = strtol(av[0], nullptr, 0);
  if(mindex > 6) return -1;

  if(ac == 1) {
    ckm_macros[mindex]->reset();
    //showmacro(s, 1, av);
    return 1;
  }

  if(ac < 3) return -1;
  uint16_t duration = strtol(av[1], nullptr, 0);
  bool simul = strtol(av[2], nullptr, 0);
  if(duration > 60000) return -1;

  uint8_t i = 0;
  ac -= 3;
  MacroEntry entry = {};
  while(i < ac && i < sizeof(entry.fcode)/sizeof(entry.fcode[0])) {
    entry.fcode[i] = (FunctionCode_t)strtol(av[3+i], nullptr, 0);
    i++;
    entry.num++;
  }

  ckm_macros[mindex]->addSequence(duration, simul, entry.fcode, entry.num);
  //showmacro(s, 1, av);
  return 1;
}

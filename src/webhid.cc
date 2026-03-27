#include <Arduino.h>
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
#include "controller.h"
#include "webhid.h"
#include "custom_keymap.h"
#include "keysw_statemachine.h"
#include "hid.h"
#include "sysutils.h"
#include "config.h"

  /*
    dont't validate keymaps because Active is validated and Default is checked when loaded.
  */
bool WebHID::fetch(webhid_info_t &req) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("WebHID fetch, primary:", req.primaryProfile, ", profile:", req.profile, ", osLayout:", req.osLayout);
    }
#endif

  bool r;

  webhid_data_t response = {};
  response.info.cmd = WEBHID_RES_ACK;
  response.info.primaryProfile = Config::getPrimaryProfileNum();

  if(!Config::validateProfileAll(req.profile)) {
    LOG_ERROR("Invalid profile", req.profile);
    req.profile = 0;
    response.info.cmd = WEBHID_RES_INVALID_PROFILE;
  }
  response.info.profile = req.profile;

  if(!CustomKeymap::validateOSLayout((KeymapOSLayoutType)req.osLayout)) {
    LOG_ERROR("Invalid layout", req.osLayout);
    req.osLayout = OSLAYOUT_US;
    response.info.cmd = WEBHID_RES_INVALID_OSLAYOUT;
  }

  Keymap &keymap_tmp = Config::fetchKeymap2tmp(r, req.profile, (KeymapOSLayoutType)req.osLayout);
  if(!r) response.info.cmd = WEBHID_RES_INVALID_SAVE;
  response.info.osLayout = keymap_tmp.layout;
  for(int layer = 0; layer < KEYMAPTABLE_MAX; layer++) {
    for(int key = 0; key < DEFS_NMAGKEYS; key++) {
      response.keymap[layer][key] = (uint8_t)keymap_tmp.ftable[layer][key];
    }
  }
  for(int unit = 0; unit < sizeof(response.funit) / sizeof(response.funit[0]); unit++) {
    response.funit[unit].key = keymap_tmp.funit[unit].key;
    response.funit[unit].state = (uint8_t)keymap_tmp.funit[unit].state;
    response.funit[unit].layer = (uint8_t)keymap_tmp.funit[unit].layer;
    response.funit[unit].func = (uint8_t)keymap_tmp.funit[unit].func;
  }
  //response.keymap.tapadv_enable = keymap_tmp.tapadv_enable;

  KeyswAttributes (&magattr_tmp)[DEFS_NMAGKEYS] = Config::fetchKeyAttributes2tmp(r, req.profile);
  if(!r) response.info.cmd = WEBHID_RES_INVALID_SAVE;
  response.magattr_pp = magattr_tmp[0].thresh_partial_press;
  response.magattr_p = magattr_tmp[0].thresh_press;
  response.magattr_pp2r = magattr_tmp[0].thresh_partial2release;
  response.magattr_p2r = magattr_tmp[0].thresh_press2release;
  response.magattr_hold_high = (uint8_t)((magattr_tmp[0].duration_hold >> 8) & 0xff);
  response.magattr_hold_low = (uint8_t)(magattr_tmp[0].duration_hold & 0xff);

  // LED
  I2CLEDAttributes &ledattr = Config::fetchLEDAttributes2tmp(r, req.profile);
  if(!r) response.info.cmd = WEBHID_RES_INVALID_SAVE;
  response.keyled_release = ledattr.led_ptn_release;
  response.keyled_partialpress = ledattr.led_ptn_partialpress;
  response.keyled_press = ledattr.led_ptn_press;
  response.keyled_error = ledattr.led_ptn_alert;
  response.brightness = ledattr.led_brightness;
  response.backlight_enable = ledattr.backlight_enable;

  ConfigKeyboardLED &keyboardled = Config::fetchConfigKeyboardLED2tmp(r, req.profile);
  if(!r) response.info.cmd = WEBHID_RES_INVALID_SAVE;
  for(uint8_t type = 0; type < LEDTYPE_MAX && type < (sizeof(response.led) / sizeof(response.led[0])); type++) {
    response.led[type].r = (keyboardled.color[type] >> 16) & 0xff;
    response.led[type].g = (keyboardled.color[type] >> 8) & 0xff;
    response.led[type].b = keyboardled.color[type] & 0xff;
  }

  ctxhid.hidmanager.webhidSendResponse((uint8_t *)&response, sizeof(response));
  return true;
}

bool WebHID::apply(webhid_data_t &data) {
  webhid_info_t response = { .cmd = WEBHID_RES_ACK };

#if defined(DEBUG)
  LOG_DEBUG("WebHID apply, osLayout:", data.info.osLayout);
  //LOG_DEBUG("primaryProfile:", data.info.primaryProfile);
  //LOG_DEBUG("profile:", data.info.profile);
  for(uint8_t layer = 0; layer < KEYMAPTABLE_MAX; layer++) {
    LOG_DEBUG("keymap[", layer, "][", 0, "]:", data.keymap[layer][0]);
    LOG_DEBUG("keymap[", layer, "][", DEFS_NMAGKEYS-1, "]:", data.keymap[layer][DEFS_NMAGKEYS-1]);
  }
  for(uint8_t unit = 0; unit < sizeof(data.funit) / sizeof(data.funit[0]); unit++) {
    LOG_DEBUG("funit[", unit, "].key:", data.funit[unit].key,
        "state:", data.funit[unit].state,
        "layer:", data.funit[unit].layer,
        "func:", data.funit[unit].func);
  }
#endif

  Keymap keymap_tmp = {};
  keymap_tmp.layout = (KeymapOSLayoutType)data.info.osLayout;
  for(int layer = 0; layer < KEYMAPTABLE_MAX; layer++) {
    for(int key = 0; key < DEFS_NMAGKEYS; key++) {
      keymap_tmp.ftable[layer][key] = (FunctionCode_t)data.keymap[layer][key];
    }
  }
  for(int unit = 0; unit < sizeof(data.funit) / sizeof(data.funit[0]); unit++) {
    if(data.funit[unit].func == 0) continue; // skip empty function unit
    keymap_tmp.funit[unit].key = data.funit[unit].key;
    keymap_tmp.funit[unit].state = (KeyswState)data.funit[unit].state;
    keymap_tmp.funit[unit].layer = (KeymapLayerType)data.funit[unit].layer;
    keymap_tmp.funit[unit].func = (FunctionCode_t)data.funit[unit].func;
  }
  keymap_tmp.tapadv_enable = true;

  if(!ctxkeymap.keymap.setKeymapByCopy(keymap_tmp)){ 
    LOG_ERROR("Invalid keymap");
    response.cmd = WEBHID_RES_INVALID_KEYMAP;
  }

  // magsw attributes
#if defined(DEBUG)
  LOG_DEBUG("magattr.pp:", data.magattr_pp,
    ", pp2r:", data.magattr_pp2r,
    ", p:", data.magattr_p,
    ", p2r:", data.magattr_p2r,
    ", hold:", data.magattr_hold_high<<8 | data.magattr_hold_low);
#endif

  KeyswAttributes magattr_tmp[DEFS_NMAGKEYS];
  for(int key = 0; key < DEFS_NMAGKEYS; key++) {
    magattr_tmp[key].thresh_partial_press = data.magattr_pp;
    magattr_tmp[key].thresh_press = data.magattr_p;
    magattr_tmp[key].thresh_partial2release = data.magattr_pp2r;
    magattr_tmp[key].thresh_press2release = data.magattr_p2r;
    magattr_tmp[key].duration_hold = data.magattr_hold_high<<8 | data.magattr_hold_low;
    magattr_tmp[key].control = KC_NONE;
  }
  if(!MagswStateMachine::validateKeyattr(magattr_tmp)) {
    LOG_ERROR("Invalid magattr data");
    response.cmd = WEBHID_RES_INVALID_MAGATTR;
  }

  for(int key = 0; key < DEFS_NMAGKEYS; key++) {
    ctxmagsw.msm[key].setKeyattr(
      magattr_tmp[key].thresh_partial_press,
      magattr_tmp[key].thresh_press,
      magattr_tmp[key].thresh_partial2release,
      magattr_tmp[key].thresh_press2release,
      magattr_tmp[key].duration_hold);
    ctxmagsw.msm[key].setCtrl(magattr_tmp[key].control);
  }

  // LED etc
#if defined(DEBUG)
  LOG_DEBUG("keyled_rel:", data.keyled_release,
    "pp:", data.keyled_partialpress,
    "p:", data.keyled_press,
    "err:", data.keyled_error);
  LOG_DEBUG(
    "led [0]", data.led[0].r, data.led[0].g, data.led[0].b,
    "led [1]", data.led[1].r, data.led[1].g, data.led[1].b,
    "led [2]", data.led[2].r, data.led[2].g, data.led[2].b,
    "led [3]", data.led[3].r, data.led[3].g, data.led[3].b,
    "led [4]", data.led[4].r, data.led[4].g, data.led[4].b,
    "led [5]", data.led[5].r, data.led[5].g, data.led[5].b,
    "led [6]", data.led[6].r, data.led[6].g, data.led[6].b);
  LOG_DEBUG("brightness:", data.brightness, "backlight_en:", data.backlight_enable);
#endif

  I2CLEDAttributes ledattr_tmp;
  ledattr_tmp.led_ptn_release = data.keyled_release;
  ledattr_tmp.led_ptn_partialpress = data.keyled_partialpress;
  ledattr_tmp.led_ptn_press = data.keyled_press;
  ledattr_tmp.led_ptn_alert = data.keyled_error;
  ledattr_tmp.led_backlight = data.brightness;
  ledattr_tmp.led_brightness = data.brightness;
  ledattr_tmp.backlight_enable = data.backlight_enable;
  if(!F_SystemLED::validateLEDAttr(ledattr_tmp)) {
    LOG_ERROR("Invalid LED attributes");
    response.cmd = WEBHID_RES_INVALID_KEYLED;
  } else {
    ctxsys.f_systemled.setKeyLEDColor(KEYLED_RELEASE, ledattr_tmp.led_ptn_release);
    ctxsys.f_systemled.setKeyLEDColor(KEYLED_PRESS, ledattr_tmp.led_ptn_press);
    ctxsys.f_systemled.setKeyLEDColor(KEYLED_PARTIALPRESS, ledattr_tmp.led_ptn_partialpress);
    ctxsys.f_systemled.setKeyLEDColor(KEYLED_ALERT, ledattr_tmp.led_ptn_alert);
    ctxsys.f_systemled.applyBrightness(ledattr_tmp.led_brightness);
    ctxsys.f_systemled.applyBacklight(ledattr_tmp.led_brightness);
    ctxsys.f_systemled.enableBacklight(ledattr_tmp.backlight_enable);
  }

  ctxsys.f_systemled.setLEDColor(LEDTYPE_CALIBRATION, data.led[0].r, data.led[0].g, data.led[0].b);
  ctxsys.f_systemled.setLEDColor(LEDTYPE_SYSTEM, data.led[1].r, data.led[1].g, data.led[1].b);
  ctxsys.f_systemled.setLEDColor(LEDTYPE_MCU, data.led[2].r, data.led[2].g, data.led[2].b);
  ctxsys.f_systemled.setLEDColor(LEDTYPE_KEY, data.led[3].r, data.led[3].g, data.led[3].b);
  ctxsys.f_systemled.setLEDColor(LEDTYPE_NUMLOCK, data.led[4].r, data.led[4].g, data.led[4].b);
  ctxsys.f_systemled.setLEDColor(LEDTYPE_CAPSLOCK, data.led[5].r, data.led[5].g, data.led[5].b);
  ctxsys.f_systemled.setLEDColor(LEDTYPE_SCROLLLOCK, data.led[6].r, data.led[6].g, data.led[6].b);

  ctxhid.hidmanager.webhidSendResponse((uint8_t *)&response, sizeof(response));

  return true;
}

bool WebHID::save(webhid_info_t &req) {
  /*
  just save the active to a profile.
  */
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("WebHID save, primary:", req.primaryProfile, ", profile:", req.profile, ", osLayout:", req.osLayout);
  }
#endif

  webhid_info_t response = { .cmd = WEBHID_RES_ACK };
  if(req.primaryProfile >= PROFILE_MAX) {
    LOG_ERROR("Invalid primary profile", req.primaryProfile);
    req.primaryProfile = 0;
    response.cmd = WEBHID_RES_INVALID_PRIMARY;
  }
  if(req.profile >= PROFILE_MAX) {
    LOG_ERROR("Invalid profile", req.profile);
    req.profile = 0;
    response.cmd = WEBHID_RES_INVALID_PROFILE;
  }

  Config::setPrimaryProfileNum(req.primaryProfile);
  if(!Config::saveConfiguration(req.profile)) {
    LOG_ERROR("Failed to save configuration");
    response.cmd = WEBHID_RES_INVALID_SAVE;
  }

  ctxhid.hidmanager.webhidSendResponse((uint8_t *)&response, sizeof(response));
  return true;
}


void WebHID::progress(webhid_data_t &data) {
/*
  if(!Config::validateProfileAll(data.profile)) {
    LOG_ERROR("Invalid profile ", data.profile, ", cmd", data.cmd, ", layout ", data.layout, ", layer ", data.layer);
    hidcontrol.initSendbuf(data.cmd, data.profile, data.layout, data.layer, 0);
    hidcontrol.send(false);
    return;

  } else if(!CustomKeymap::validateLayer(data.layer)) {
    LOG_ERROR("Invalid layer ", data.layer, ", cmd", data.cmd, ", profile ", data.profile, ", layout ", data.layout);
    hidcontrol.initSendbuf(data.cmd, data.profile, data.layout, data.layer, 0);
    hidcontrol.send(false);
    return;
  }
*/

  switch(data.info.cmd) {
    case WEBHID_CMD_FETCH:
      fetch(data.info);
      break;

    case WEBHID_CMD_APPLY:
      apply(data);
      break;

    case WEBHID_CMD_SAVE:
      save(data.info);
      break;

    default:
      LOG_ERROR("Unknown command ", (int)data.info.cmd);
      break;
  }
}

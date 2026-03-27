#include <Arduino.h>
#include <Stream.h>
//#include <Adafruit_TinyUSB.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "hid.h"
#include "sysutils.h"
#include "functions.h"


appcontext_hid_t &appctxHid() {
  static appcontext_hid_t ctx = {
    .hidmanager = HidManager(),
    .debug_hidcallback = CircularBuffer(),
    .error_hidcallback = CircularBuffer(),
  };
  return ctx;
}

void HidManager::begin() {
  composite.hid.setBootProtocol(HID_ITF_PROTOCOL_NONE);
  composite.hid.enableOutEndpoint(true);
  composite.hid.setStringDescriptor(DEFS_HID_COMPOSITE_NAME);
  composite.hid.setReportCallback(getReportCallbackComposite, setReportCallbackComposite);
  composite.hid.setReportDescriptor(report_desc_composite, sizeof(report_desc_composite));
  composite.hid.setPollInterval(2);
  composite.instance_num = composite.hid.getInstanceCount();
  composite.hid.begin();

  boot_keyboard.hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  boot_keyboard.hid.enableOutEndpoint(true);
  boot_keyboard.hid.setStringDescriptor(DEFS_HID_BOOTKEYBOARD_NAME);
  boot_keyboard.hid.setReportCallback(getReportCallbackBootkbd, setReportCallbackBootkbd);
  boot_keyboard.hid.setReportDescriptor(report_desc_bootkbd, sizeof(report_desc_bootkbd));
  boot_keyboard.hid.setPollInterval(2);
  boot_keyboard.instance_num = boot_keyboard.hid.getInstanceCount();
  boot_keyboard.hid.begin();

#if defined(DEBUG)
  ctxhid.debug_hidcallback.print("[DEBUG] HID begin. composite:");
  ctxhid.debug_hidcallback.print(composite.instance_num);
  ctxhid.debug_hidcallback.print(", bootkbd:");
  ctxhid.debug_hidcallback.print(boot_keyboard.instance_num);
  ctxhid.debug_hidcallback.print('\n');
#endif
}

bool HidManager::waitHidReady(hid_instance_t &instance) {
  if(TinyUSBDevice.suspended()) {
    TinyUSBDevice.remoteWakeup();
  }
  while(!instance.hid.ready() || !instance.report_complete) {
    instance.not_ready_count++;
    if(instance.not_ready_count % 1000) { LOG_ERROR("HID not ready"); }
  }

  /*
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("instance", instance.instance_num, ", not_ready_count=", instance.not_ready_count);
  }
#endif
  */

  instance.not_ready_count = 0;
  return true;
}

bool HidManager::hidSendReport(hid_instance_t &instance, uint8_t report_id, void const *report, uint8_t len) {
  //TinyUSBDevice.task(); // for better USB latency, call task() before HID reporting.
  waitHidReady(instance);
  instance.report_complete = false;
  bool r = instance.hid.sendReport(report_id, report, len);
  if(!r) instance.report_complete = true;

  return(r);
}

bool HidManager::keyboardSendReport() {
  bool r;

  memcpy(&kbd_report_prev, &kbd_report, sizeof(kbd_report)); // save current report for next comparison.

  if(kbd_protocol_feature) {
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("code[] = 0x", DebugLogBase::HEX,
        kbd_report.keycode[0], kbd_report.keycode[1], kbd_report.keycode[2], kbd_report.keycode[3],
        kbd_report.keycode[4], kbd_report.keycode[5], kbd_report.keycode[6], kbd_report.keycode[7],
        kbd_report.keycode[8], kbd_report.keycode[9], kbd_report.keycode[10], kbd_report.keycode[11],
        " mod = 0x", kbd_report.modifier);
    }
    if(ctxsys.kb_ctrl.isKeyboardReport())
#endif
    {
      r = hidSendReport(composite, HID_RID_KEYBOARD, &kbd_report, sizeof(kbd_report));
    }
  }
  
  else {
#ifdef DEBUG
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("code = ",
        kbd_report_boot.keycode[0],kbd_report_boot.keycode[1],kbd_report_boot.keycode[2],
        kbd_report_boot.keycode[3],kbd_report_boot.keycode[4],kbd_report_boot.keycode[5],
        " mod = ", kbd_report_boot.modifier);
    }
    if(ctxsys.kb_ctrl.isKeyboardReport())
#endif
    {
      r = hidSendReport(boot_keyboard, 0, &kbd_report_boot, sizeof(kbd_report_boot));
    }
  }

  return r;
}

bool HidManager::addKeycode(uint8_t code, uint8_t mod) {
#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("code 0x", DebugLogBase::HEX, code, ", mod 0x", mod);
  }
#endif

  kbd_report.keycode[HID_NKRO_BYTE(code)] |= HID_NKRO_BIT(code);
  kbd_report.modifier |= mod;

  if(kbd_report_boot_index < 6) {
    kbd_report_boot.keycode[kbd_report_boot_index++] = code;
  }
  kbd_report_boot.modifier |= mod;

  return true;
}

bool HidManager::mouseSendReport(uint8_t btn, int8_t x, int8_t y, int8_t vertical, int8_t horizontal) {
  hid_report_mouse_t mouseReport = {btn, x, y, vertical, horizontal};
  bool r = true;

#ifdef DEBUG
  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode()) {
    LOG_DEBUG("btn/x/y/v/h = ",btn, x, y, vertical, horizontal);
  }
  if(ctxsys.kb_ctrl.isMouseReport())
#endif
  {
    r = hidSendReport(composite, HID_RID_MOUSE, &mouseReport, sizeof(mouseReport));
  }

  return r;
}

bool HidManager::tpmouseSendReport(TPPoint &left, TPPoint &right) {
  int16_t x_diff, y_diff;
  if(left.touch) {
    x_diff = (int16_t)left.x_filtered - (int16_t)mm_x_l;
    mm_x_l = left.x_filtered;
    y_diff = (int16_t)left.y_filtered - (int16_t)mm_y_l;
    mm_y_l = left.y_filtered;
  } else if(right.touch) {
    x_diff = (int16_t)right.x_filtered - (int16_t)mm_x_r;
    mm_x_r = right.x_filtered;
    y_diff = (int16_t)right.y_filtered - (int16_t)mm_y_r;
    mm_y_r = right.y_filtered;
  } else {
    x_diff = y_diff = 0;
  }

  hid_report_mouse_t mouseReport = {
    (uint8_t)((left.btn ? MOUSE_BUTTON_LEFT : 0) | (right.btn ? MOUSE_BUTTON_RIGHT : 0)),
    (int8_t)(x_diff / 12), // scale down to fit int8_t
    (int8_t)(y_diff / 12), // scale down to fit int8_t
    0, 0};

  bool r = false;
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isTouchpadReport())
#endif
  {
    r = hidSendReport(composite, HID_RID_MOUSE, &mouseReport, sizeof(mouseReport));
  }
  return r;
}

bool HidManager::tpmouseScroll(int8_t distance) {
  hid_report_mouse_t mouseReport = {
    0, // no button
    0, // no movement
    0, // no movement
    distance, 0};

  bool r = false;
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isTouchpadReport())
#endif
  {
    r = hidSendReport(composite, HID_RID_MOUSE, &mouseReport, sizeof(mouseReport));
  }
  return r;
}

bool HidManager::touchpadSendReport(bool firsttouch, TPPoint &left, TPPoint &right) {
  unsigned long now_100us;
  if(firsttouch) {
    scantime_reference = micros()/100;
    now_100us = scantime_reference;
  } else {
    now_100us = micros()/100;
  }
  unsigned long diff_100us = now_100us - scantime_reference;

  bool debug = false;

  /* Fixed touch ID
    Touch Left:  1
    Touch Right: 2
    Button Left: 3
    Button Right:4

    current HID Reprot definition supports 3 touches.
    priority:
    1. Left/Right touch
    2. Button Left with touch
    3. Button Right with touch, if touch report is not full
    */

  /* improvements
    2. if no touch no button, report count 0 report. -> control state machine in touchpad::childaction().
    3. double tap improvement
      a. repor tip=1, tip=0, count=0, tip=1
      //b. different contact ID for each tap.
  */

  hid_report_touchpad_t tp_report = {};
  uint8_t index = 0, count = 0;

  /*
    touch : TipSwitch = 1, Report contact with x/y, count++
    releaseing : TipSwitch = 0, Report contact with last x/y, no count
    all have just released : count = 0 report
  */
  switch(left.touch_stat) {
    case TP_STAT_RELEASING:
      setTouchParam(tp_report, false, index++, 1, left.x_filtered, left.y_filtered);
      debug = true;
      count++;
      break;
    case TP_STAT_RELEASE: // do nothing
      break;
    case TP_STAT_TOUCH: // for touch start
      debug = true;
    default: // TP_STAT_TOUCH, TP_STAT_TOUCHING
      setTouchParam(tp_report, true, index++, 1, left.x_filtered, left.y_filtered);
      count++;
      break;
  }

  switch(right.touch_stat) {
    case TP_STAT_RELEASING:
      setTouchParam(tp_report, false, index++, 2, right.x_filtered, right.y_filtered);
      debug = true;
      count++;
      break;
    case TP_STAT_RELEASE: // do nothing
      break;
    case TP_STAT_TOUCH: // for touch start
      debug = true;
    default: // TP_STAT_TOUCH, TP_STAT_TOUCHING
      setTouchParam(tp_report, true, index++, 2, right.x_filtered, right.y_filtered);
      count++;
      break;
  }

  switch(left.btn_stat) {
    case TP_STAT_RELEASING:
      setTouchParam(tp_report, false, index++, 3, 10, HID_TP_LOGICAL_Y - 10); // TipSwitch off
      debug = true;
      count++;
      break;
    case TP_STAT_RELEASE: // do nothing
      break;
    case TP_STAT_TOUCH: // for button press start
      debug = true;
    default: // TP_STAT_TOUCH, TP_STAT_TOUCHING
      setTouchParam(tp_report, true, index++, 3, 10, HID_TP_LOGICAL_Y - 10);
      count++;
      tp_report.base.contact_count_buttons |= TP_BUTTON2;
      break;
  }

  switch(right.btn_stat) {
    case TP_STAT_RELEASING:
      setTouchParam(tp_report, false, index++, 4, HID_TP_LOGICAL_X - 10, HID_TP_LOGICAL_Y - 10); // TipSwitch off
      debug = true;
      count++;
      break;
    case TP_STAT_RELEASE: // do nothing
      break;
    case TP_STAT_TOUCH: // for button press start
      debug = true;
    default: // TP_STAT_TOUCH, TP_STAT_TOUCHING
      if(index < TP_MAX_CONTACTS) {
        setTouchParam(tp_report, true, index++, 4, HID_TP_LOGICAL_X - 10, HID_TP_LOGICAL_Y - 10);
        count++;
        tp_report.base.contact_count_buttons |= TP_BUTTON3;
      }
      break;
  }

  tp_report.base.contact_count_buttons |= ((count & TP_CONTACT_COUNT_MASK) << TP_CONTACT_COUNT_SHIFT);
  tp_report.base.scan_time = diff_100us;

  bool r = true;
#if defined(DEBUG)
  if((debug || count == 0) && ctxsys.kb_ctrl.isDebugMode()) {
    printTouchpadReport(tp_report);
  }
  if(ctxsys.kb_ctrl.isTouchpadReport())
#endif
  {
    r = hidSendReport(composite, HID_RID_TP_TOUCH, &tp_report, sizeof(tp_report));
  }

  return r;
}

bool HidManager::webhidReceiveRequest(uint8_t *buf, size_t len) {
  if(webhid_req_bufinfo.buf == nullptr || webhid_req_bufinfo.len == 0) {
    LOG_ERROR("WebHID request buffer not set");
    return false;
  }

  if(!buf) {
    LOG_ERROR("buf is null");
    return false;
  }

  if(!webhid_req_bufinfo.complete) {
    return false;
  }

  if(len < webhid_req_bufinfo.index) {
    LOG_ERROR("buffer size", len, "too small, required", webhid_req_bufinfo.index);
    return false;
  }

  memcpy(buf, webhid_req_bufinfo.buf, webhid_req_bufinfo.index);
  webhid_req_bufinfo.index = 0;
  webhid_req_bufinfo.complete = false;
  return true;
}

bool HidManager::webhidSendResponse(uint8_t *buf, size_t len) {
  if(!buf || len == 0) return false;

  if(webhid_res_bufinfo.buf == nullptr || webhid_res_bufinfo.len == 0) {
    LOG_ERROR("WebHID response buffer not set");
    return false;
  }

  if(len > webhid_res_bufinfo.len) {
    LOG_ERROR("WebHID response buffer lack. len", webhid_res_bufinfo.len,
      ", data len", len);
    return false;
  }

#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("WebHID send response, len ", len, " data: ", (int)buf[0], (int)buf[1], (int)buf[2], (int)buf[3]);
  }
#endif
  memcpy(webhid_res_bufinfo.buf, buf, len);
  webhid_res_bufinfo.index = 0;
  webhid_res_bufinfo.stored_len = len;
  webhid_res_bufinfo.complete = true;

  return(true);
}

void HidManager::setReportCallbackBootkbd(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buf, uint16_t bufsize) {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.print("[DEBUG] setReportCallbackBootkbd report_id:");
    ctxhid.debug_hidcallback.print(report_id);
    ctxhid.debug_hidcallback.print(", report_type:");
    ctxhid.debug_hidcallback.print(report_type);
    ctxhid.debug_hidcallback.print(", bufsize:");
    ctxhid.debug_hidcallback.print(bufsize);
    ctxhid.debug_hidcallback.print(", buf[0]:");
    ctxhid.debug_hidcallback.print(buf[0]);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("report_id:", report_id, ", report_type:", report_type, ", bufsize:", bufsize, ", buf[0]:", buf[0]);
  }
#endif

  uint8_t bm = buf[0] & 0x7;
  if(kbd_led_bm != bm) {
    kbd_led_bm = bm;
    kbd_led_changed = true;
  }
}

uint16_t HidManager::getReportCallbackBootkbd(uint8_t report_id, hid_report_type_t report_type, uint8_t* buf, uint16_t bufsize) {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.print("[DEBUG] getReportCallbackBootkbd report_id:");
    ctxhid.debug_hidcallback.print(report_id);
    ctxhid.debug_hidcallback.print(", report_type:");
    ctxhid.debug_hidcallback.print(report_type);
    ctxhid.debug_hidcallback.print(", bufsize:");
    ctxhid.debug_hidcallback.print(bufsize);
    ctxhid.debug_hidcallback.print(", buf[0]:");
    ctxhid.debug_hidcallback.print(buf[0]);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("report_id:", report_id, ", report_type:", report_type, ", bufsize:", bufsize, ", buf[0]:", buf[0]);
  }
#endif
  return 0;
}

void HidManager::setReportCallbackComposite(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buf, uint16_t bufsize) {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.print("[DEBUG] setReportCallbackComposite report_id:");
    ctxhid.debug_hidcallback.print(report_id);
    ctxhid.debug_hidcallback.print(", report_type:");
    ctxhid.debug_hidcallback.print(report_type);
    ctxhid.debug_hidcallback.print(", bufsize:");
    ctxhid.debug_hidcallback.print(bufsize);
    ctxhid.debug_hidcallback.print(", buf[0]:");
    ctxhid.debug_hidcallback.print(buf[0]);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("report_id:", report_id, ", report_type:", report_type, ", bufsize:", bufsize, ", buf[0]:", buf[0]);
  }
#endif

  if(report_id == HID_RID_KEYBOARD) {
    uint8_t bm = buf[0] & 0x7;
    if(kbd_led_bm != bm) {
      kbd_led_bm = bm;
      kbd_led_changed = true;
    }
  }

  /*
  https://learn.microsoft.com/ja-jp/windows-hardware/design/component-guidelines/touchpad-configuration-collection?source=recommendations
  */
  else if(report_id == HID_RID_TP_DEVICEMODE) {
    // buf[0] == 3
  }

  else if(report_id == HID_RID_TP_SWITCH) {
    // buf[0] == 3
    // SURFACE_SWITCH | BUTTON_SWITCH
  }

  else if(report_id == HID_RID_WEBHID) {
    // handled in getReportCallbackComposite()
    if(webhid_req_bufinfo.buf == nullptr || webhid_req_bufinfo.len == 0) {
      ctxhid.error_hidcallback.print("[ERROR] setReportCallbackComposite: WebHID request buffer not set\n");
      return;
    }

    webhid_fragment_t *fragment = (webhid_fragment_t *)buf;
    // The data in buf should be stored at the position of webhid_req_bufinfo.index.
    // Do not copy if it exceeds the length of buf.
    size_t copy_len = fragment->len;
    if (webhid_req_bufinfo.index + copy_len > webhid_req_bufinfo.len) {
      copy_len = webhid_req_bufinfo.len - webhid_req_bufinfo.index;
    }
    if (copy_len > 0) {
      memcpy(webhid_req_bufinfo.buf + webhid_req_bufinfo.index, fragment->payload, copy_len);
      webhid_req_bufinfo.index += copy_len;
    }
    // If fragment->fragment_type is 0, it means the last fragment, so set complete = true.
    if (fragment->type == 0) {
      webhid_req_bufinfo.complete = true;
    }
  }
}

uint16_t HidManager::getReportCallbackComposite(uint8_t report_id, hid_report_type_t report_type, uint8_t* buf, uint16_t bufsize) {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.print("[DEBUG] getReportCallbackComposite report_id:");
    ctxhid.debug_hidcallback.print(report_id);
    ctxhid.debug_hidcallback.print(", report_type:");
    ctxhid.debug_hidcallback.print(report_type);
    ctxhid.debug_hidcallback.print(", bufsize:");
    ctxhid.debug_hidcallback.print(bufsize);
    ctxhid.debug_hidcallback.print(", buf[0]:");
    ctxhid.debug_hidcallback.print(buf[0]);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("report_id:", report_id, ", report_type:", report_type, ", bufsize:", bufsize, ", buf[0]:", buf[0]);
  }
#endif

  if(report_id == HID_RID_TP_MAXCOUNT) {
    hid_report_tp_maxcount_t maxcount_report = { TP_PADTYPE_CLICK | ((TP_MAX_CONTACTS & TP_MAX_CONTACTS_MASK) << TP_MAX_CONTACTS_SHIFT) };
    size_t len = sizeof(maxcount_report); // report ID is added in hidd_control_xfer_cb()
    if(bufsize < len) len = bufsize;
    memcpy(buf, &maxcount_report, len);
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      ctxhid.debug_hidcallback.print("    MAXCOUNT report len:");
      ctxhid.debug_hidcallback.print(len);
      ctxhid.debug_hidcallback.print(", max_count:");
      ctxhid.debug_hidcallback.print(maxcount_report.max_count);
      ctxhid.debug_hidcallback.print('\n');
      //LOG_DEBUG("    MAXCOUNT report len", len, ", max_count", maxcount_report.max_count);
    }
#endif
    return len;
  }

  else if(report_id == HID_RID_TP_PTPHQA) {
    return 0;
  }

  else if(report_id == HID_RID_WEBHID) {
    if(webhid_res_bufinfo.buf == nullptr || webhid_res_bufinfo.len == 0) {
      ctxhid.error_hidcallback.print("[ERROR] getReportCallbackComposite: WebHID response buffer not set\n");
      return 0;

    } else if(bufsize < sizeof(webhid_fragment_t)) {
      ctxhid.error_hidcallback.print("[ERROR] getReportCallbackComposite: Buffer size too small for WebHID fragment. bufsize:");
      ctxhid.error_hidcallback.print(bufsize);
      ctxhid.error_hidcallback.print(", required:");
      ctxhid.error_hidcallback.print(sizeof(webhid_fragment_t));
      ctxhid.error_hidcallback.print('\n');
      return 0;

    } else if(!webhid_res_bufinfo.complete) {
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        ctxhid.debug_hidcallback.print("[DEBUG] getReportCallbackComposite: WebHID response buffer not complete.\n");
      }
#endif
      return 0;

    } else {
      webhid_fragment_t *fragment = (webhid_fragment_t *)buf;
      size_t copy_len = webhid_res_bufinfo.stored_len - webhid_res_bufinfo.index;
      if (copy_len > sizeof(fragment->payload)) {
        copy_len = sizeof(fragment->payload);
      }
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        ctxhid.debug_hidcallback.print("[DEBUG] getReportCallbackComposite: WebHID response copy_len:");
        ctxhid.debug_hidcallback.print(copy_len);
        ctxhid.debug_hidcallback.print('\n');
      }
#endif
      memcpy(fragment->payload, webhid_res_bufinfo.buf + webhid_res_bufinfo.index, copy_len);
      webhid_res_bufinfo.index += copy_len;
      if (webhid_res_bufinfo.index >= webhid_res_bufinfo.stored_len) {
        webhid_res_bufinfo.complete = false;
        webhid_res_bufinfo.index = 0;
      }
      fragment->len = copy_len;
      fragment->type = (webhid_res_bufinfo.complete) ? WEBHID_FRAGMENT_CONT : WEBHID_FRAGMENT_END;
      return sizeof(webhid_fragment_t);
    }
  }

  return 0;
}

void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.print("[DEBUG] tud_hid_set_protocol_cb instance:");
    ctxhid.debug_hidcallback.print(instance);
    ctxhid.debug_hidcallback.print(", protocol:");
    ctxhid.debug_hidcallback.print(protocol);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("tud_hid_set_protocol_cb instance:", instance, ", protocol:", protocol);
  }
#endif

  if(protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    HidManager::kbd_protocol_feature = false;
  } else {
    HidManager::kbd_protocol_feature = true;
  }
}

bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate) {
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    ctxhid.debug_hidcallback.print("[DEBUG] tud_hid_set_idle_cb instance:");
    ctxhid.debug_hidcallback.print(instance);
    ctxhid.debug_hidcallback.print(", idle_rate:");
    ctxhid.debug_hidcallback.print(idle_rate);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("tud_hid_set_idle_cb instance:", instance, ", idle_rate:", idle_rate);
  }
#endif
  return true;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
  if(ctxhid.hidmanager.composite.instance_num == instance) {
    ctxhid.hidmanager.composite.report_complete = true;
    //ctxhid.debug_hidcallback.print("[DEBUG] Report complete. instance:");
    //ctxhid.debug_hidcallback.print(instance);
    //ctxhid.debug_hidcallback.print('\n');
  }
  else if(ctxhid.hidmanager.boot_keyboard.instance_num == instance) {
    ctxhid.hidmanager.boot_keyboard.report_complete = true;
    //ctxhid.debug_hidcallback.print("[DEBUG] Report complete. instance:");
    //ctxhid.debug_hidcallback.print(instance);
    //ctxhid.debug_hidcallback.print('\n');
  }
  else {
    ctxhid.error_hidcallback.print("[ERROR] Unknown instance in tud_hid_report_complete_cb: ");
    ctxhid.error_hidcallback.print(instance);
    ctxhid.error_hidcallback.print('\n');
    return;
  }
/*
#if defined(DEBUG)
  static unsigned long last = 0;
  unsigned long now = millis();
  unsigned long diff = now - last;

  if(ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isSlowMode() && diff > 1000) {
    ctxhid.debug_hidcallback.print("[DEBUG] tud_hid_report_complete_cb instance:");
    ctxhid.debug_hidcallback.print(instance);
    ctxhid.debug_hidcallback.print(", report[0]:");
    ctxhid.debug_hidcallback.print(*report);
    ctxhid.debug_hidcallback.print(", len:");
    ctxhid.debug_hidcallback.print(len);
    ctxhid.debug_hidcallback.print('\n');
    //LOG_DEBUG("tud_hid_report_complete_cb instance:", instance, ", report[0]:", *report, ", len:", len);
    last = now;
  }
#endif
*/
}

void tud_hid_report_failed_cb(uint8_t instance, hid_report_type_t report_type, uint8_t const* report, uint16_t xferred_bytes) {
  ctxhid.error_hidcallback.print("[ERROR] tud_hid_report_failed_cb instance:");
  ctxhid.error_hidcallback.print(instance);
  ctxhid.error_hidcallback.print(", report_type:");
  ctxhid.error_hidcallback.print(report_type);
  ctxhid.error_hidcallback.print(", report[0]");
  ctxhid.error_hidcallback.print(*report);
  ctxhid.error_hidcallback.print(", xferred_bytes:");
  ctxhid.error_hidcallback.print(xferred_bytes);
  ctxhid.error_hidcallback.print('\n');
  //LOG_DEBUG("tud_hid_report_failed_cb instance:", instance, ", report_type:", report_type, ", report[0]", *report, ", xferred_bytes:", xferred_bytes);
}

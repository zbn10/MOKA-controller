#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "functions.h"
#include "hid.h"
#include "sysutils.h"


void F_Touchpad::testCircle() {
  unsigned long now = micros();
  unsigned long diff = now - circle_last_time;

  if(diff < 4000) return;

  circle_last_time = now;

  /*
  hid_report_touchpad_t report;
  memset(&report, 0, sizeof(report));
  if(toggle) {
    report.contacts[0].stat = TP_STAT_TOUCH_VALID | TP_STAT_TIP_SWITCH | (1 << TP_CONTACT_ID_SHIFT);
    float angle = PI * 2 * c / TEST_Hz;
    report.contacts[0].x = 2048 + sin(angle) * 1000;
    report.contacts[0].y = 2048 + cos(angle) * 1000;
    report.base.contact_count_buttons = 1;
  }
  */

  if(toggle) {
    float angle = PI * 2.0 * circle / TEST_Hz;
    left.x_input = sin(angle) * 400 + 1200;
    left.y_input = cos(angle) * 400 + 800;
    left.touch = true;
  }
  circle = (circle + 1) % TEST_Hz;
  if(circle == 0) {
    toggle = !toggle;
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Toggle to ", toggle);
    }
#endif
  }

  return;
}

void F_Touchpad::testVScroll() {
  unsigned long now = micros();
  unsigned long diff = now - processed_time;

  if(diff < 8000) return;

  circle = (circle + 1) % TEST_Hz;
}

void F_Touchpad::oneEuroFilter(bool firsttouch, unsigned long diff_us, TPPoint &p, oef_param_t &oef) {
  /*
  one euro filter
  dt = 5ms or more
  x[n] = xl / conversion_factor // logical scale -> mm
  tau(f) = 1 / (2 * pi * f)
  alpha(f) = dt / (dt + tau(f)) = 2 * pi * f * dt / (1 + 2 * pi * f * dt)
  vxraw[n] = (x[n] - x[n-1]) / dt
  vx[n] = alpha(fd) * vxraw[n] + (1 - alpha(fd)) * vx[n-1]
  v[n] = sqrt(vx[n] * vx[n] + vy[n] * vy[n])
  fc[n] = fmin + beta * v[n]
  X[n] = alpha(fc[n]) * x[n] + (1 - alpha(fc[n])) * X[n-1]
  xl_filtered = X[n] * conversion_factor // mm -> logical scale

  fd : cutoff for acceleration, 1.0 Hz
  fmin : minimum cutoff, 1.5 Hz, reduce jitter when slow
  beta : speed coefficient, 0.002 ~ 0.08 s/mm, high tracking ability when fast

  First, tune the low speed reange
  1. find an acceptable lower bound for fmin with beta = 0.
  2. fd is basically around 1Hz. Move it up and down a little to see the touchpad movement.
  3. beta value is small.

  Next, tune the high speed range
  4. Increase the beta value and see how it goes.
  5. When beta adjustment is not effective, increase fmin slightly. Increasing fmin has a slow range impact.
   */
/*
one euro filter
dt = 5ms or more
x[n] = xl / conversion_factor // logical scale -> mm
tau(f) = 1 / (2 * pi * f)
alpha(f) = dt / (dt + tau(f)) = 2 * pi * f * dt / (1 + 2 * pi * f * dt)
vxraw[n] = (x[n] - x[n-1]) / dt
vx[n] = alpha(fd) * vxraw[n] + (1 - alpha(fd)) * vx[n-1]
v[n] = sqrt(vx[n] * vx[n] + vy[n] * vy[n])
fc[n] = fmin + beta * v[n]
X[n] = alpha(fc[n]) * x[n] + (1 - alpha(fc[n])) * X[n-1]
xl_filtered = X[n] * conversion_factor // mm -> logical scale

fd : cutoff for acceleration, 1.0 Hz
fmin : minimum cutoff, 1.5 Hz, reduce jitter when slow
beta : speed coefficient, 0.002 ~ 0.08 s/mm, high tracking ability when fast

First, tune the low speed reange
1. find an acceptable lower bound for fmin with beta = 0.
2. fd is basically around 1Hz. Move it up and down a little to see the touchpad movement.
3. beta value is small.

Next, tune the high speed range
4. Increase the beta value and see how it goes.
5. When beta adjustment is not effective, increase fmin slightly. Increasing fmin has a slow range impact.
*/

  if(firsttouch) {
    p.x_filtered = p.x_input;
    p.y_filtered = p.y_input;

    p.x_filtered_mm_prev = p.x_input / conversion_factor;
    p.y_filtered_mm_prev = p.y_input / conversion_factor;
    p.vx_prev = 0; // because vxraw is zero in this case.
    p.vy_prev = 0;
  }

#define ALPHA(f) (2 * PI * (f) * (dt) / (1 + 2 * PI * (f) * (dt)))
//  else {
    float dt = diff_us / 1000000.0;
    float x_mm = p.x_input / conversion_factor;
    float y_mm = p.y_input / conversion_factor;
    float vxraw = (x_mm - p.x_filtered_mm_prev) / dt;
    float vyraw = (y_mm - p.y_filtered_mm_prev) / dt;
    float alpha_fd = ALPHA(oef.fd);
    float vx = alpha_fd * vxraw + (1 - alpha_fd) * (p.vx_prev);
    float vy = alpha_fd * vyraw + (1 - alpha_fd) * (p.vy_prev);
    float v = sqrt(vx * vx + vy * vy);
    float fc = oef.fmin + oef.beta * v;
    float alpha_c = ALPHA(fc);
    p.x_filtered_mm_prev = alpha_c * x_mm + (1 - alpha_c) * p.x_filtered_mm_prev;
    p.y_filtered_mm_prev = alpha_c * y_mm + (1 - alpha_c) * p.y_filtered_mm_prev;
    p.x_filtered = p.x_filtered_mm_prev * conversion_factor;
    p.y_filtered = p.y_filtered_mm_prev * conversion_factor;

    p.vx_prev = vx;
    p.vy_prev = vy;
//  }
}

ActionResult F_Touchpad::childaction(unsigned long ms, FunctionCode_t fc) {
  if(fc == F_NONE) {
    unsigned long us = micros();
    unsigned long diff_us = us - processed_time;

    if(diff_us < (ctxsys.kb_ctrl.isDebugMode() && ctxsys.kb_ctrl.isTouchpadReport() ? 100000 : TP_REPORT_INTERVAL_US)) return F_RES_NONE;
    /*
      In the normal situation report shold be sent every HID interval set by HID.setPollInterval().
      otherwise, it may cause touch detection on the OS may be interrupted.
    */

    /* state machine: state of touch and button are the same. not use 'tipsw off state' of touch.
      state       touch/button
      RELEASE
                  finger touch
      TOUCH
                  finger release
      RELEASING -> TipSwitch=0 report
                  no touch
      RELEASE -> count 0 report once
      */

    auto touchTransition = [](TPPoint &tpp) {
      switch(tpp.touch_stat) {
        case TP_STAT_RELEASE:
          if(tpp.touch) tpp.touch_stat = TP_STAT_TOUCH;
          break;
        case TP_STAT_TOUCH:
          tpp.touch_stat = (tpp.touch ? TP_STAT_TOUCHING : TP_STAT_RELEASING);
          break;
        case TP_STAT_TOUCHING:
          if(!tpp.touch) tpp.touch_stat = TP_STAT_RELEASING;
          break;
        case TP_STAT_RELEASING:
          tpp.touch_stat = TP_STAT_RELEASE;
          break;
      }
    };

    auto btnTransition = [](TPPoint &tpp) {
      switch(tpp.btn_stat) {
        case TP_STAT_RELEASE:
          if(tpp.btn) tpp.btn_stat = TP_STAT_TOUCHING;
          break;
        case TP_STAT_TOUCHING:
          if(!tpp.btn) tpp.btn_stat = TP_STAT_RELEASING;
          break;
        case TP_STAT_RELEASING:
          tpp.btn_stat = TP_STAT_RELEASE;
          break;
      }
    };

    // Scroll
    touchTransition(scroll);
    if(scroll.touch_stat == TP_STAT_TOUCH || scroll.touch_stat == TP_STAT_TOUCHING) {
      float angle = atan2(scroll.y_filtered - 800, scroll.x_filtered - 800);

      if(scroll.touch_stat == TP_STAT_TOUCH) { // first touch
        oneEuroFilter(true, diff_us, scroll, oef_left);
        scroll.angle_prev = angle;
        scroll.x_filtered_prev = scroll.x_filtered;
        scroll.y_filtered_prev = scroll.y_filtered;
        scroll.count = 0;
      } else {
        oneEuroFilter(false, diff_us, scroll, oef_left);
      }

      float delta = angle - scroll.angle_prev;
      if(delta > PI) delta -= 2 * PI;
      else if(delta < -PI) delta += 2 * PI;
      scroll.angle_prev = angle;

      float diff_x = scroll.x_filtered - scroll.x_filtered_prev;
      float diff_y = scroll.y_filtered - scroll.y_filtered_prev;
      scroll.x_filtered_prev = scroll.x_filtered;
      scroll.y_filtered_prev = scroll.y_filtered;
      uint16_t distance = (uint16_t)sqrt(diff_x * diff_x + diff_y * diff_y);

      uint8_t report_interval = (uint8_t)(200 / (distance+1));

      if(distance > 5 && scroll.count > report_interval) {
        ctxhid.hidmanager.tpmouseScroll(delta > 0 ? -1 : 1); // continuous reporting is not needed because of using mouse wheel report.
        scroll.count = 0;
      } else scroll.count++;
      scroll.touch = false;
      processed_time = us;
      /*
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG("Scroll delta", delta, ", distance", distance);
      }
#endif
      */
      return F_RES_SUCCESS;
    }

    // Left touch
    // if in TP_STAT_RELEASING, call touchpadSendREport with previous filtered value.
    touchTransition(left);
    if(left.touch_stat == TP_STAT_TOUCH) {
      oneEuroFilter(true, diff_us, left, oef_left);
    } else if(left.touch_stat == TP_STAT_TOUCHING) {
      oneEuroFilter(false, diff_us, left, oef_left);
    }
    // Left button
    btnTransition(left);

    // Right touch
    touchTransition(right);
    if(right.touch_stat == TP_STAT_TOUCH) {
      oneEuroFilter(true, diff_us, right, oef_right);
    } else if(right.touch_stat == TP_STAT_TOUCHING) {
      oneEuroFilter(false, diff_us, right, oef_right);
    }
    // Right button
    btnTransition(right);


    if(left.touch_stat == TP_STAT_RELEASE
      && left.btn_stat == TP_STAT_RELEASE
      && right.touch_stat == TP_STAT_RELEASE
      && right.btn_stat == TP_STAT_RELEASE) {
      vpad.touch = false;
    } else {
      vpad.touch = true;
    }
    touchTransition(vpad);

    if(vpad.touch_stat != TP_STAT_RELEASE) {
      /*
#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG("left touch/btn stat", left.touch_stat, left.btn_stat, " xf:", left.x_filtered, ", yf:", left.y_filtered,
          "right touch/btn stat", right.touch_stat, right.btn_stat, " xf:", right.x_filtered, ", yf:", right.y_filtered);
      }
#endif
      */

      if(mouse_mode) ctxhid.hidmanager.tpmouseSendReport(left, right);
      else ctxhid.hidmanager.touchpadSendReport(vpad.touch_stat == TP_STAT_TOUCH, left, right);

      processed_time = us;
      left.touch = right.touch = left.btn = right.btn = false;
      return F_RES_SUCCESS;
    }

    left.touch = right.touch = left.btn = right.btn = false;
    return F_RES_NONE;
  }

  else if(fc == F_TOUCHPAD_LEFT) {
    // normal position : Left (1600, 800) -> (3200, 2400), Right (3200, 800) -> (4800, 2400)
    // virtual pad size : 3200 x 3200 + 3200 x 3200
    left.touch = TP_GET_TOUCH(inputvalue32);
    left.x_input = TP_GET_X(inputvalue32)/2 + 1600;
    left.y_input = TP_GET_Y(inputvalue32)/2 + 800;
    /*
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && left.touch) {
      LOG_DEBUG("TP LEFT value32:0x", DebugLogBase::HEX, inputvalue32, ", xl:", DebugLogBase::DEC, left.x_input, ", yl:", left.y_input);
    }
#endif
    */
  }
  else if(fc == F_TOUCHPAD_LEFT2X) {
    // 2x position : Left (0, 0) -> (3200, 3200), Right (3200, 000) -> (6400, 3200)
    // virtual pad size : 3200 x 3200 + 3200 x 3200
    left.touch = TP_GET_TOUCH(inputvalue32);
    left.x_input = TP_GET_X(inputvalue32);
    left.y_input = TP_GET_Y(inputvalue32);
    /*
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && left.touch) {
      LOG_DEBUG("TP LEFT2X value32:0x", DebugLogBase::HEX, inputvalue32, ", xl:", DebugLogBase::DEC, left.x_input, ", yl:", left.y_input);
    }
#endif
    */
  }

  else if(fc == F_TOUCHPAD_RIGHT) {
    right.touch = TP_GET_TOUCH(inputvalue32);
    right.x_input = TP_GET_X(inputvalue32)/2 + 3200;
    right.y_input = TP_GET_Y(inputvalue32)/2 + 800;
    /*
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && right.touch) {
      LOG_DEBUG("TP RIGHT value32:0x", DebugLogBase::HEX, inputvalue32, ", xr:", DebugLogBase::DEC, right.x_input, ", yr:", right.y_input);
    }
#endif
    */
  } else if(fc == F_TOUCHPAD_RIGHT2X) {
    right.touch = TP_GET_TOUCH(inputvalue32);
    right.x_input = TP_GET_X(inputvalue32) + 3200;
    right.y_input = TP_GET_Y(inputvalue32);
      /*
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode() && right.touch) {
      LOG_DEBUG("TP RIGHT2X value32:0x", DebugLogBase::HEX, inputvalue32, ", xr:", DebugLogBase::DEC, right.x_input, ", yr:", right.y_input);
    }
#endif
    */
  }

  else if(fc == F_TOUCHPAD_BTN_LEFT) {
    left.btn = true;
  }
  
  else if(fc == F_TOUCHPAD_BTN_RIGHT) {
    right.btn = true;
  }

  else if(fc == F_TOUCHPAD_MOUSE) {
    if(!beginning) return F_RES_NONE;
    mouse_mode = !mouse_mode;
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Touchpad Mouse mode ->", (mouse_mode ? "ON" : "OFF"));
    }
#endif
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1,
        "TPad Mode", (mouse_mode ? "Mouse" : "Touchpad"));
  }

  else if(fc == F_TOUCHPAD_ENABLE) {
    if(!beginning) return F_RES_NONE;
    bool enable = ctxsys.kb_ctrl.isTouchpadReport();
    enable  = !enable;
    ctxsys.kb_ctrl.enableTouchpadReport(enable);
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("Touchpad HID report ->", (enable ? "ON" : "OFF"));
    }
#endif
    ctxsys.ev_handler.pushNewEvent(SystemEvent::PR_PRIORITY, SystemEvent::GR_NONE, 1,
        "TPad Enabled", (enable ? "ON" : "OFF"));
  }

  else if(fc == F_JIGGLER) {
    testCircle();
  }

  else if(fc == F_TOUCHPAD_SCROLL) {
    scroll.touch = TP_GET_TOUCH(inputvalue32);
    scroll.x_input = TP_GET_X(inputvalue32)/2;
    scroll.y_input = TP_GET_Y(inputvalue32)/2;
  }

  return F_RES_SUCCESS;
}


bool F_Touchpad::validateOEFParam(const oef_param_t &param) {
  return (param.fd > 0.0f && param.fd < 5.0f && param.fmin > 0.0f && param.fmin < 5.0f && param.beta >= 0.0f && param.beta < 1.0f);
}

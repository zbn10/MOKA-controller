#pragma once
#include <Arduino.h>
#include "controller.h"
#include "keysw_statemachine.h"
#include "functions.h"

#define CK_TACTSW 42
// 
#if defined(SUPPORT_TINYMOUSE)
#define CK_JOYSTICKMOUSE 43
#endif
//
#if defined(SUPPORT_TOUCHPAD)
#define CK_TOUCHPAD_LEFT 44
#define CK_TOUCHPAD_RIGHT 45
#endif
//
#if defined(SUPPORT_PHOTOIC_INFRARED)
#define CK_PHOTOIC 46
#define CK_INFRAREDSENSOR 47
#endif

// layout
enum KeymapOSLayoutType : uint8_t {
  OSLAYOUT_US = 0,
  OSLAYOUT_JP,
  OSLAYOUT_MAX
};

enum KeymapLayerType : uint8_t {
  // keymap.ftable[][] has 3 layers.
  LAYER_0 = 0,
  LAYER_PRESS = 0,
  LAYER_1,
  LAYER_2,
  LAYER_3,
  LAYER_CALIBRATION,
  LAYER_EMERGENCY,
  LAYER_MAX,
  KEYMAPTABLE_MAX = LAYER_3 + 1, // keymap.ftable[KEYMAPTABLE_MAX]
};

struct FunctionUnit {
  uint8_t key;
  KeyswState state;
  KeymapLayerType layer;
  FunctionCode_t func;
};

struct Keymap { // profile save target
  /*
    permanent layer on ftable[][] and editor : PRESS, USER LAYER0, USER LAYER1
    individual layer : PARTIAL, HOLD, LONGHOLD, ...
    key function : layer + key
  */

  KeymapOSLayoutType layout;
  FunctionCode_t ftable[KEYMAPTABLE_MAX][DEFS_NMAGKEYS]; // PRESS, LAYER0, LAYER1
  FunctionUnit funit[DEFS_FUNITS]; // other layersunc
  /*
    F_CALIBRATION : Pseudo_Key_Calibration PRESS -> F_LAYER_CALIBRATION
    key, PRESS or HOLD, F_LAYER_CALIBRATION : F_CALIBRATION off
    funit[1] = {tactsw, LONGHOLD1, 0, F_CALIBRATION}
    funit[0] = {tactsw, PRESS, F_LAYER_CALIBRATION, F_CALIBRATION}
    funit[2] = {key, PRESS, 0, F_LAYER0}
  */

  bool tapadv_enable; // used with keymap
};

struct FunctionInstance {
  FunctionCode_t fcode;
  Functions *func[OSLAYOUT_MAX];
  const char *descr;
  const char *disp;
};

class CustomKeymap {
public:
  CustomKeymap(Keymap &km) : keymap(km) {}
  static bool setupFunctionInstances();
  bool setOSLayout(KeymapOSLayoutType layout);
  KeymapOSLayoutType getOSLayout() { return keymap.layout; }
  const char *getOSLayoutStr() { return(keymap.layout == OSLAYOUT_US ? "US" : "JP"); }
  bool setFunctionCode(uint8_t key, KeyswState state, KeymapLayerType layer, FunctionCode_t keycode); // for keymap.ftable[] and keymap.funit[]
  FunctionCode_t getFunctionCode(uint8_t key, KeyswState state, KeymapLayerType layer); // for keymap.ftable[], keymap.funit[] and funit_common[]
  FunctionCode_t getKeymapTable(uint8_t key, KeymapLayerType layer) { // for webhid
    if(!validateKey(key) || !validateLayer(layer)) return F_NONE;
    return(keymap.ftable[layer][key]);
  }
  FunctionUnit &getFunctionUnit(uint8_t pos) { // for webhid
    if(pos < sizeof(keymap.funit) / sizeof(keymap.funit[0])) { return keymap.funit[pos]; }
    return empty;
  }
  Keymap &getKeymap() { return keymap; }
  bool setKeymapByCopy(Keymap &km) {
    keymap = km;
    bool r = validateKeymap(keymap);
    optimizeKeymap();
    return r;
  }
  bool isTapAdvanced() { return keymap.tapadv_enable; }
  void enableTapAdvanced(bool e) {
    keymap.tapadv_enable = e;
  }

  void resetFunctionUnit() { num_funit = 0; }
  Functions *getFunctionInstance(FunctionCode_t code) {
    return(f_instances[code].func[keymap.layout]);
  }
  const char *getFunctionDescr(FunctionCode_t code) {
    return(f_instances[code].descr);
  }
  const char *getFunctionDisp(FunctionCode_t code) {
    return(f_instances[code].disp);
  }

  bool optimizeKeymap() {
    return(alignFunctionUnit());
  }
  static bool validateOSLayout(uint8_t layout) { return(layout < OSLAYOUT_MAX); }
  static bool validateLayer(uint8_t layer) { return(layer < LAYER_MAX); }
  static bool validateTableLayer(uint8_t state) { return(state < KEYMAPTABLE_MAX); }
  static bool validateKey(uint8_t key) { return(key < DEFS_NKEYS); }
  static bool validateFunctionCode(uint8_t key, KeyswState state, KeymapLayerType layer, FunctionCode_t keycode);
  static bool validateKeymap(Keymap &km);
  static bool fetchFunctionKeylist(uint16_t index, FunctionCode_t &fcode, const char *&disp);

  static const char *macroCmd;
  static const char *macroSyntax;
  static const char *macroDescr;
  static int macro(Stream *s, uint8_t ac, const char **av);

  static KeyswStateMachine *ksm[DEFS_NKEYS];
  static const FunctionUnit funit_common[];
  static const uint8_t num_funit_common;
  inline static FunctionUnit empty = {0, KS_RELEASE, LAYER_0, F_NONE};

private:
  Keymap &keymap;
  uint8_t num_funit;

  FunctionCode_t searchInTables(uint8_t key, KeyswState state, KeymapLayerType layer);
  FunctionCode_t searchInUnits(uint8_t key, KeyswState state, KeymapLayerType layer);
  bool alignFunctionUnit();

  static FunctionInstance f_instances_setting[256];
  inline static FunctionInstance f_instances[256];
  static FunctionInstance f_instances_back[256];
  static FunctionCode_t keylist[14*13];
};


class F_SelectLayer : public Functions {
public:
  F_SelectLayer(FunctionCode_t f, KeymapLayerType l) :
    Functions(f, (f == F_NONE ? F_LC_CONTINUOUS : F_LC_ONCE), (f == F_NONE ? F_PR_LOW : F_PR_MIDDLE)), target_layer(l) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
  static uint8_t enterCalibration();
  static uint8_t exitCalibration();
  static KeymapLayerType getSelectedLayer() { return selected_layer; }
  static void setSelectedLayer(KeymapLayerType l) { selected_layer = l; }
private:
  bool running = false;
  const KeymapLayerType target_layer;
  inline static KeymapLayerType selected_layer = LAYER_0;
  inline static KeymapLayerType holded_layer = LAYER_0;
  inline static bool set_holded_layer = false;
  inline static bool holding = false;  // used for keeping HOLD LAYER in the F_NONE instance
};

class F_ToggleKeymapOSLayout : public Functions {
public:
  F_ToggleKeymapOSLayout(FunctionCode_t f) : Functions(f, F_LC_ONCE, F_PR_LOW) {}
  void begin() override {}
  ActionResult childaction(unsigned long, FunctionCode_t) override;
};

typedef struct {
  CustomKeymap keymap;
  F_SelectLayer f_layer_handler;
} appcontext_keymap_t;
appcontext_keymap_t &appctxKeymap();
inline appcontext_keymap_t &ctxkeymap = appctxKeymap();

#pragma once
#include "controller.h"
#include "i2c_control.h"
#include "mcu.h"


class Utils {
public:
  inline static uint8_t scan_result[2][127];
  inline static const char *display_str, *display_str2;
  inline static I2CLoopData mculoop;
  inline static I2CKeyTestData keytestdata;

  static void i2cScanner(Stream *s);
  static void getMemUsage(Stream *s);

  static const char *calibrationCmd;
  static const char *calibrationSyntax;
  static const char *calibrationDescr;
  static int calibration(Stream *s, uint8_t ac, const char **av);

  static const char *keyvalCmd;
  static const char *keyvalSyntax;
  static const char *keyvalDescr;
  static int keyval(Stream *s, uint8_t ac, const char **av);

  static const char *ledCmd;
  static const char *ledSyntax;
  static const char *ledDescr;
  static int led(Stream *s, uint8_t ac, const char **av);

  static const char *sysstatCmd;
  static const char *sysstatSyntax;
  static const char *sysstatDescr;
  static int sysstat(Stream *s, uint8_t ac, const char **av);

  static const char *keycodeCmd;
  static const char *keycodeSyntax;
  static const char *keycodeDescr;
  static int keycode(Stream *s, uint8_t ac, const char **av);

  static const char *keyattrCmd;
  static const char *keyattrSyntax;
  static const char *keyattrDescr;
  static int keyattr(Stream *s, uint8_t ac, const char **av);

  static const char *resetMCUEEPROMCmd;
  static const char *resetMCUEEPROMSyntax;
  static const char *resetMCUEEPROMDescr;
  static int resetMCUEEPROM(Stream *s, uint8_t ac, const char **av);

  static const char *emergencyCmd;
  static const char *emergencySyntax;
  static const char *emergencyDescr;
  static int emergency(Stream *s, uint8_t ac, const char **av);

  static const char *verCmd;
  static const char *verSyntax;
  static const char *verDescr;
  static int ver(Stream *s, uint8_t ac, const char **av);

#if defined(DEBUG)
  static const char *eventcliCmd;
  static const char *eventcliSyntax;
  static const char *eventcliDescr;
  static int eventcli(Stream *s, uint8_t ac, const char **av);

  static const char *debugCmd;
  static const char *debugSyntax;
  static const char *debugDescr;
  static int debug(Stream *s, uint8_t ac, const char **av);

  static const char *pdebugCmd;
  static const char *pdebugSyntax;
  static const char *pdebugDescr;
  static int pdebug(Stream *s, uint8_t ac, const char **av);

  static const char *slowCmd;
  static const char *slowSyntax;
  static const char *slowDescr;
  static int slow(Stream *s, uint8_t ac, const char **av);

  static const char *hidrptCmd;
  static const char *hidrptSyntax;
  static const char *hidrptDescr;
  static int hidrpt(Stream *s, uint8_t ac, const char **av);
  
  static const char *tpparamCmd;
  static const char *tpparamSyntax;
  static const char *tpparamDescr;
  static int tpparam(Stream *s, uint8_t ac, const char **av);

  static const char *getmsgCmd;
  static const char *getmsgSyntax;
  static const char *getmsgDescr;
  static int getmsg(Stream *s, uint8_t ac, const char **av);

  static const char *loopCmd;
  static const char *loopSyntax;
  static const char *loopDescr;
  static int loop(Stream *s, uint8_t ac, const char **av);

  static const char *keytestCmd;
  static const char *keytestSyntax;
  static const char *keytestDescr;
  static int keytest(Stream *s, uint8_t ac, const char **av);
#endif
};

class timeMeasurement {
public:
  timeMeasurement() : index(0), lock(false) {}
  void measurement(uint8_t i = 0) {
    if(lock) return;
    us[index] = micros();
    id[index] = i;
    index++;
    if(index >= LOOP_MAX) {
      index = 0;
    }
  }
  void showResults(Stream *s) {
    lock = true;
    delay(3);
    uint8_t i = index;
    uint8_t i_next = (i + 1) % LOOP_MAX;
    for(uint8_t l = 1; l < LOOP_MAX; l++) {
      s->print("loop ");
      s->print((int)i);
      s->print(" (id:");
      s->print((int)id[i]);
      s->print(") : ");
      s->print(us[i_next] - us[i]);
      s->print(" us\n");
      i = (i + 1) % LOOP_MAX;
      i_next = (i_next + 1) % LOOP_MAX;
    }
    lock = false;
  }

private:
  static constexpr uint8_t LOOP_MAX = 11;
  unsigned long us[LOOP_MAX];
  uint8_t id[LOOP_MAX];
  uint8_t index;
  bool lock;
};

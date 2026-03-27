#pragma once
#include "i2c_control.h"
#include "keysw_statemachine.h"

#define MCU_NMCUS      6



class KeySensorMCU {
  public:
    KeySensorMCU(uint8_t n, uint8_t addr) : mcu_number(n), i2c_addr(addr) {}
    uint8_t getAddr() { return i2c_addr; }
    uint8_t getMCUNumber() { return mcu_number; }
    void setError(void);
    bool getError(void) { return error; }
    uint8_t isAlive(void);

    int setLEDState(uint8_t state1, uint8_t state2);
    //int setLEDAlert(uint8_t alert);
    int calibrationPress(uint8_t);
    int resetEEPROM();
    static int resetEEPROMAll();

    int getRawval(uint8_t, uint16_t (&)[I2CCtrl::NKEYS]);
    int getSeveralData(I2CMCUData *);
    int showMCUMessage(Stream *s);
    int getMCUVersion(I2CVersionInfo &vinfo);

    static void reflectKeyStateOnLED(KeyswStateMachine **ksm);
    static int setLEDAttributesAll(I2CLEDAttributes &la);
    static int calibrationMode(bool);
    static int calibrationReleaseAll();

    static uint8_t *i2cdata;

    static const uint8_t keypos2mcu[DEFS_NMAGKEYS];
    static const uint8_t keypos2key[DEFS_NMAGKEYS];
    static const uint8_t mcukey2keypos[MCU_NMCUS][I2CCtrl::NKEYS];

  private:
    uint8_t mcu_number;
    const uint8_t i2c_addr;
    bool error = false;
};

typedef struct {
  KeySensorMCU mcu[MCU_NMCUS];
} appcontext_mcu_t;
appcontext_mcu_t &appctxMcu();
inline appcontext_mcu_t &ctxmcu = appctxMcu();

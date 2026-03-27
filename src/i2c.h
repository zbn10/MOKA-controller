#pragma once
#include "sysutils.h"

typedef struct {
  CircularBuffer debug_loop1;
  timeMeasurement i2cloop;
} appcontext_i2c_t;
appcontext_i2c_t &appctxI2C();
inline appcontext_i2c_t &ctxi2c = appctxI2C();

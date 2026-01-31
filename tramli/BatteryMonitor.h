#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "Config.h"

struct BatteryState {
  int percentage;
  float voltage;
  int rawAdc;
};

void initBattery();
void updateBatteryState(BatteryState& state);

#endif

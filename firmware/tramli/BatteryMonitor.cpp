#include "BatteryMonitor.h"

void initBattery() {
  pinMode(BATTERY_ADC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
}

void updateBatteryState(BatteryState& state) {
  // Use 4 samples without blocking delays for power efficiency
  // The ADC settles fast enough that delays aren't necessary
  long sum = 0;
  for (int i = 0; i < 4; i++) {
    sum += analogReadMilliVolts(BATTERY_ADC_PIN);
  }
  int avgMilliVolts = sum / 4;

  // Multiply by 2 because of voltage divider
  float voltage = (avgMilliVolts / 1000.0) * 2.0;

  state.rawAdc = avgMilliVolts;
  state.voltage = voltage;

  if (voltage > 4.2) voltage = 4.2;
  if (voltage < 3.2) voltage = 3.2;

  state.percentage = (int)((voltage - 3.2) / (4.2 - 3.2) * 100.0);
}

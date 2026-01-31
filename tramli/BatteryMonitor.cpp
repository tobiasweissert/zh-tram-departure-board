#include "BatteryMonitor.h"

void initBattery() {
  pinMode(BATTERY_ADC_PIN, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);
}

void updateBatteryState(BatteryState& state) {
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(BATTERY_ADC_PIN);
    delay(1);
  }
  int avgReading = sum / 16;

  float voltage = (avgReading / 4095.0) * 3.3 * 2.0;

  state.rawAdc = avgReading;
  state.voltage = voltage;

  if (voltage > 4.2) voltage = 4.2;
  if (voltage < 3.2) voltage = 3.2;

  state.percentage = (int)((voltage - 3.2) / (4.2 - 3.2) * 100.0);
}

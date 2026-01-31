#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include "DataStructures.h"
#include "BatteryMonitor.h"

bool initDisplay(Adafruit_SSD1306& display);
void displayDepartures(Adafruit_SSD1306& display,
                      Departure* currentDepartures,
                      int currentCount,
                      bool offlineMode,
                      const BatteryState& batteryState);

#endif

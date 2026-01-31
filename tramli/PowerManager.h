#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

void displayOff(Adafruit_SSD1306& display);
void displayOn(Adafruit_SSD1306& display);
void enterDeepSleep();
void saveStateBeforeSleep(bool currentShowBus72);
void restoreStateAfterWake(bool& showBus72);

#endif

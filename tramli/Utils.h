#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <time.h>

unsigned long getCurrentTimestamp();
String getCurrentTimeString();
String convertUmlauts(String text);

void initNVS();
void saveTimeToNVS(time_t timestamp);
time_t loadTimeFromNVS();

#endif

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <time.h>

unsigned long getCurrentTimestamp();
void getCurrentTimeString(char* buffer, size_t bufferSize);
void convertUmlauts(char* text, size_t bufferSize);

void initNVS();
void saveTimeToNVS(time_t timestamp);
time_t loadTimeFromNVS();

#endif

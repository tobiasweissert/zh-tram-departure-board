#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <Arduino.h>

struct Departure {
  char destination[32];  // Fixed buffer to avoid heap fragmentation
  char number[8];        // Fixed buffer for line number (e.g., "7", "72")
  unsigned long departureTimestamp;
};

#endif

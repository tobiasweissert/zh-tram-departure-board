#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <Arduino.h>

struct Departure {
  String destination;
  String number;
  unsigned long departureTimestamp;
};

#endif

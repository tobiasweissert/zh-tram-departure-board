#ifndef OFFLINE_SCHEDULE_H
#define OFFLINE_SCHEDULE_H

#include <Arduino.h>
#include "DataStructures.h"
#include "ScheduleData.h"

void getOfflineDepartures(Departure tram7Departures[], int& tram7Count,
                          Departure bus72Departures[], int& bus72Count);

#endif

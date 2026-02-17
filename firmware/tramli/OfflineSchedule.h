#ifndef OFFLINE_SCHEDULE_H
#define OFFLINE_SCHEDULE_H

#include <Arduino.h>
#include "DataStructures.h"
#include "ScheduleData.h"

void getOfflineDepartures(Departure tramDepartures[], int& tramCount,
                          Departure busDepartures[], int& busCount);

#endif

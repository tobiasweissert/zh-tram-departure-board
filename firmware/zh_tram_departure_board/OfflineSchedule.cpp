#include "OfflineSchedule.h"
#include "Config.h"
#include <time.h>
#include <string.h>

void getOfflineDepartures(Departure tramDepartures[], int& tramCount,
                          Departure busDepartures[], int& busCount) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;
  int currentDayOfWeek = (timeinfo->tm_wday + 6) % 7;
  int currentTimeMinutes = currentHour * 60 + currentMinute;

  tramCount = 0;
  busCount = 0;
  int lastTramTime = -1;

  for (int i = 0; i < tramScheduleSize && tramCount < 5; i++) {
    if (!scheduleAppliesToday(tramSchedule[i].dayMask, currentDayOfWeek)) {
      continue;
    }

    int entryTimeMinutes = tramSchedule[i].hour * 60 + tramSchedule[i].minute;

    if (entryTimeMinutes >= currentTimeMinutes - 1) {
      // Skip duplicate times caused by overlapping day masks
      if (entryTimeMinutes == lastTramTime) {
        continue;
      }
      lastTramTime = entryTimeMinutes;

      Departure dep;
      strncpy(dep.number, TRAM_LINE_NUMBER, sizeof(dep.number) - 1);
      dep.number[sizeof(dep.number) - 1] = '\0';
      strncpy(dep.destination, tramSchedule[i].destination, sizeof(dep.destination) - 1);
      dep.destination[sizeof(dep.destination) - 1] = '\0';

      struct tm depTime = *timeinfo;
      depTime.tm_hour = tramSchedule[i].hour;
      depTime.tm_min = tramSchedule[i].minute;
      depTime.tm_sec = 0;
      depTime.tm_isdst = -1;  // Let mktime determine DST
      dep.departureTimestamp = mktime(&depTime);

      tramDepartures[tramCount++] = dep;
    }
  }

  int lastBusTime = -1;

  for (int i = 0; i < busScheduleSize && busCount < 5; i++) {
    if (!scheduleAppliesToday(busSchedule[i].dayMask, currentDayOfWeek)) {
      continue;
    }

    int entryTimeMinutes = busSchedule[i].hour * 60 + busSchedule[i].minute;

    if (entryTimeMinutes >= currentTimeMinutes - 1) {
      // Skip arrivals at own station (end station), only show departures
      if (strcmp(busSchedule[i].destination, STATION_DISPLAY_NAME) == 0) {
        continue;
      }

      // Skip duplicate times caused by overlapping day masks
      if (entryTimeMinutes == lastBusTime) {
        continue;
      }
      lastBusTime = entryTimeMinutes;

      Departure dep;
      strncpy(dep.number, BUS_LINE_NUMBER, sizeof(dep.number) - 1);
      dep.number[sizeof(dep.number) - 1] = '\0';
      strncpy(dep.destination, busSchedule[i].destination, sizeof(dep.destination) - 1);
      dep.destination[sizeof(dep.destination) - 1] = '\0';

      struct tm depTime = *timeinfo;
      depTime.tm_hour = busSchedule[i].hour;
      depTime.tm_min = busSchedule[i].minute;
      depTime.tm_sec = 0;
      depTime.tm_isdst = -1;  // Let mktime determine DST
      dep.departureTimestamp = mktime(&depTime);

      busDepartures[busCount++] = dep;
    }
  }
}

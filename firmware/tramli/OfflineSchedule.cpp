#include "OfflineSchedule.h"
#include <time.h>
#include <string.h>

void getOfflineDepartures(Departure tram7Departures[], int& tram7Count,
                          Departure bus72Departures[], int& bus72Count) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;
  int currentDayOfWeek = (timeinfo->tm_wday + 6) % 7;
  int currentTimeMinutes = currentHour * 60 + currentMinute;

  tram7Count = 0;
  bus72Count = 0;
  int lastTram7Time = -1;

  for (int i = 0; i < tram7ScheduleSize && tram7Count < 5; i++) {
    if (!scheduleAppliesToday(tram7Schedule[i].dayMask, currentDayOfWeek)) {
      continue;
    }

    int entryTimeMinutes = tram7Schedule[i].hour * 60 + tram7Schedule[i].minute;

    if (entryTimeMinutes >= currentTimeMinutes - 1) {
      // Skip duplicate times caused by overlapping day masks
      if (entryTimeMinutes == lastTram7Time) {
        continue;
      }
      lastTram7Time = entryTimeMinutes;

      Departure dep;
      strncpy(dep.number, "7", sizeof(dep.number) - 1);
      dep.number[sizeof(dep.number) - 1] = '\0';
      strncpy(dep.destination, tram7Schedule[i].destination, sizeof(dep.destination) - 1);
      dep.destination[sizeof(dep.destination) - 1] = '\0';

      struct tm depTime = *timeinfo;
      depTime.tm_hour = tram7Schedule[i].hour;
      depTime.tm_min = tram7Schedule[i].minute;
      depTime.tm_sec = 0;
      depTime.tm_isdst = -1;  // Let mktime determine DST
      dep.departureTimestamp = mktime(&depTime);

      tram7Departures[tram7Count++] = dep;
    }
  }

  int lastBus72Time = -1;

  for (int i = 0; i < bus72ScheduleSize && bus72Count < 5; i++) {
    if (!scheduleAppliesToday(bus72Schedule[i].dayMask, currentDayOfWeek)) {
      continue;
    }

    int entryTimeMinutes = bus72Schedule[i].hour * 60 + bus72Schedule[i].minute;

    if (entryTimeMinutes >= currentTimeMinutes - 1) {
      // Skip buses arriving at Morgental (end station), only show departures
      if (strcmp(bus72Schedule[i].destination, "Morgental") == 0) {
        continue;
      }

      // Skip duplicate times caused by overlapping day masks
      if (entryTimeMinutes == lastBus72Time) {
        continue;
      }
      lastBus72Time = entryTimeMinutes;

      Departure dep;
      strncpy(dep.number, "72", sizeof(dep.number) - 1);
      dep.number[sizeof(dep.number) - 1] = '\0';
      strncpy(dep.destination, bus72Schedule[i].destination, sizeof(dep.destination) - 1);
      dep.destination[sizeof(dep.destination) - 1] = '\0';

      struct tm depTime = *timeinfo;
      depTime.tm_hour = bus72Schedule[i].hour;
      depTime.tm_min = bus72Schedule[i].minute;
      depTime.tm_sec = 0;
      depTime.tm_isdst = -1;  // Let mktime determine DST
      dep.departureTimestamp = mktime(&depTime);

      bus72Departures[bus72Count++] = dep;
    }
  }
}

// ScheduleData.example.h - Example schedule data for offline mode
// Copy to ScheduleData.h, or generate real data using tools/generate_schedules.py
//
// This file contains a minimal example with Paradeplatz (Tram 2 + Tram 8).
// Generate your own ScheduleData.h with real GTFS data for your station.

#ifndef SCHEDULE_DATA_H
#define SCHEDULE_DATA_H

// Schedule entry structure
struct ScheduleTime {
  unsigned char hour;      // 0-23
  unsigned char minute;    // 0-59
  unsigned char dayMask;   // Bit mask: 0=Mon, 1=Tue, 2=Wed, 3=Thu, 4=Fri, 5=Sat, 6=Sun
                          // 0x1F = weekdays (Mon-Fri)
                          // 0x7F = all days
                          // 0x60 = weekend (Sat-Sun)
  const char* destination;
};

// Tram Schedule (example: Tram 2 from Paradeplatz)
// Replace with real data using generate_schedules.py
const ScheduleTime tramSchedule[] = {
  { 6,  0, 0x1F, "Farbhof"},
  { 6,  7, 0x1F, "Farbhof"},
  { 6, 15, 0x1F, "Farbhof"},
  { 6, 22, 0x1F, "Farbhof"},
  { 6, 30, 0x1F, "Farbhof"},
  { 7,  0, 0x60, "Farbhof"},
  { 7, 15, 0x60, "Farbhof"},
  { 7, 30, 0x60, "Farbhof"},
};

const int tramScheduleSize = 8;

// Bus Schedule (example: Tram 8 from Paradeplatz)
// Replace with real data using generate_schedules.py
const ScheduleTime busSchedule[] = {
  { 6,  3, 0x1F, "Hardturm"},
  { 6, 10, 0x1F, "Hardturm"},
  { 6, 18, 0x1F, "Hardturm"},
  { 6, 25, 0x1F, "Hardturm"},
  { 6, 33, 0x1F, "Hardturm"},
  { 7,  3, 0x60, "Hardturm"},
  { 7, 18, 0x60, "Hardturm"},
  { 7, 33, 0x60, "Hardturm"},
};

const int busScheduleSize = 8;

// Helper function to check if schedule applies to current day
inline bool scheduleAppliesToday(unsigned char dayMask, int dayOfWeek) {
  // dayOfWeek: 0=Monday, 6=Sunday
  return (dayMask & (1 << dayOfWeek)) != 0;
}

#endif // SCHEDULE_DATA_H

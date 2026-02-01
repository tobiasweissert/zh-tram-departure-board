#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include "DataStructures.h"

bool connectWiFi();
bool fetchDepartures(Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count,
                     bool forceRefresh = false);
bool parseDepartures(String json, Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count);

// Check if current time is during offline hours (no service)
bool isOfflineHours();

#endif

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include "DataStructures.h"

bool connectWiFi();
bool fetchDepartures(Departure tramDepartures[], int& tramCount,
                     Departure busDepartures[], int& busCount,
                     bool forceRefresh = false);
bool parseDepartures(String json, Departure tramDepartures[], int& tramCount,
                     Departure busDepartures[], int& busCount);

// Check if current time is during offline hours (no service)
bool isOfflineHours();

#endif

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include "DataStructures.h"

bool connectWiFi();
bool fetchDepartures(Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count);
bool parseDepartures(String json, Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count);

#endif

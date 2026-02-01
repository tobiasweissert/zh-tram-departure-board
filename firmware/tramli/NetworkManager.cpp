#include "NetworkManager.h"
#include "Config.h"
#include "Utils.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>

// Response cache for avoiding redundant API calls
static unsigned long lastFetchTimestamp = 0;
static const unsigned long CACHE_TTL_MS = 30000;  // 30-second cache TTL

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.disconnect(false);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Exponential backoff: start 100ms, double each retry, cap at 4000ms, max 8 attempts
  int attempts = 0;
  unsigned long backoffMs = 100;
  const unsigned long maxBackoffMs = 4000;
  const int maxAttempts = 8;

  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(backoffMs);
    attempts++;
    if (backoffMs < maxBackoffMs) {
      backoffMs *= 2;
      if (backoffMs > maxBackoffMs) backoffMs = maxBackoffMs;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Use MAX_MODEM for deeper power savings between transmissions
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
  }
  return (WiFi.status() == WL_CONNECTED);
}

bool fetchDepartures(Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count,
                     bool forceRefresh) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  // Check cache TTL - skip fetch if recent data exists (unless forced)
  unsigned long currentMillis = millis();
  if (!forceRefresh && lastFetchTimestamp > 0 &&
      (currentMillis - lastFetchTimestamp) < CACHE_TTL_MS) {
    return true;  // Use cached data
  }

  HTTPClient http;
  http.begin(API_URL);
  http.setTimeout(15000);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    bool result = parseDepartures(payload, tram7Departures, tram7Count,
                                   bus72Departures, bus72Count);
    payload = "";
    http.end();

    if (result) {
      lastFetchTimestamp = currentMillis;
    }
    return result;
  } else {
    http.end();
    return false;
  }
}

bool parseDepartures(String json, Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count) {
  StaticJsonDocument<8192> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    return false;
  }

  if (!doc.containsKey("stationboard")) {
    return false;
  }

  JsonArray stationboard = doc["stationboard"].as<JsonArray>();
  bus72Count = 0;
  tram7Count = 0;
  unsigned long currentTime = getCurrentTimestamp();

  for (JsonObject item : stationboard) {
    Departure dep;
    const char* destStr = item["to"] | "";
    const char* numStr = item["number"] | "";
    strncpy(dep.destination, destStr, sizeof(dep.destination) - 1);
    dep.destination[sizeof(dep.destination) - 1] = '\0';
    strncpy(dep.number, numStr, sizeof(dep.number) - 1);
    dep.number[sizeof(dep.number) - 1] = '\0';
    dep.departureTimestamp = item["stop"]["departureTimestamp"].as<unsigned long>();

    long secondsUntil = dep.departureTimestamp - currentTime;
    int minutesUntil = secondsUntil / 60;

    if (minutesUntil < -1) {
      continue;
    }

    if (strcmp(dep.number, "72") == 0) {
      if (bus72Count < 5) {
        bus72Departures[bus72Count] = dep;
        bus72Count++;
      }
    }
    else if (strcmp(dep.number, "7") == 0 &&
             strstr(dep.destination, "Wollishoferplatz") == nullptr) {
      if (tram7Count < 5) {
        tram7Departures[tram7Count] = dep;
        tram7Count++;
      }
    }
  }

  return (bus72Count > 0 || tram7Count > 0);
}

bool isOfflineHours() {
  // Check if current time is during offline hours (1:00 AM - 4:30 AM)
  // No public transport service during these hours
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  int currentMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

  // Offline hours: 01:00 (60 min) to 04:30 (270 min)
  return (currentMinutes >= 60 && currentMinutes < 270);
}

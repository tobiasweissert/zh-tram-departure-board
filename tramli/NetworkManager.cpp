#include "NetworkManager.h"
#include "Config.h"
#include "Utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.disconnect(false);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }

  return (WiFi.status() == WL_CONNECTED);
}

bool fetchDepartures(Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  HTTPClient http;
  http.begin(API_URL);
  http.setTimeout(15000);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    bool result = parseDepartures(payload, tram7Departures, tram7Count,
                                   bus72Departures, bus72Count);
    payload = "";
    http.end();
    return result;
  } else {
    http.end();
    return false;
  }
}

bool parseDepartures(String json, Departure tram7Departures[], int& tram7Count,
                     Departure bus72Departures[], int& bus72Count) {
  StaticJsonDocument<12288> doc;
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
    dep.destination = item["to"].as<String>();
    dep.number = item["number"].as<String>();
    dep.departureTimestamp = item["stop"]["departureTimestamp"].as<unsigned long>();

    long secondsUntil = dep.departureTimestamp - currentTime;
    int minutesUntil = secondsUntil / 60;

    if (minutesUntil < -1) {
      continue;
    }

    if (dep.number == "72") {
      if (bus72Count < 5) {
        bus72Departures[bus72Count] = dep;
        bus72Count++;
      }
    }
    else if (dep.number == "7" &&
             dep.destination.indexOf("Wollishoferplatz") < 0) {
      if (tram7Count < 5) {
        tram7Departures[tram7Count] = dep;
        tram7Count++;
      }
    }
  }

  return (bus72Count > 0 || tram7Count > 0);
}

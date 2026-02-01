#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPI.h>
#include <driver/gpio.h>

#include "Config.h"
#include "Icons.h"
#include "DataStructures.h"
#include "BatteryMonitor.h"
#include "Display.h"
#include "NetworkManager.h"
#include "Utils.h"
#include "OfflineSchedule.h"
#include "PowerManager.h"

// RTC Memory Variables (persist across light sleep)
RTC_DATA_ATTR uint32_t bootCount = 0;
RTC_DATA_ATTR time_t rtcLastTimestamp = 0;
RTC_DATA_ATTR bool rtcShowBus72 = true;
RTC_DATA_ATTR bool rtcDisplayDisabled = false;

// Global Variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK,
                         OLED_DC, OLED_RESET, OLED_CS);

Departure bus72Departures[5];
Departure tram7Departures[5];
int bus72Count = 0;
int tram7Count = 0;

bool showBus72 = true;
bool displayDisabled = false;
unsigned long lastToggle = 0;
unsigned long lastUpdate = 0;
unsigned long lastBatteryUpdate = 0;
unsigned long lastNVSSave = 0;
bool offlineMode = false;

BatteryState batteryState;

void setup() {
  bootCount++;

  if(!initDisplay(display)) {
    while(1);
  }

  display.clearDisplay();
  display.cp437(true);
  display.setTextColor(SSD1306_WHITE);
  display.setFont();
  display.setTextSize(1);

  initBattery();
  updateBatteryState(batteryState);
  initNVS();

  // Release GPIO hold from deep sleep and configure for toggle switch
  gpio_hold_dis((gpio_num_t)BUTTON_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Set timezone immediately (CET/CEST)
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();

  time_t restoredTime = 0;
  if (rtcLastTimestamp > 0) {
    restoredTime = rtcLastTimestamp;
  } else {
    restoredTime = loadTimeFromNVS();
  }

  if (restoredTime > 0) {
    struct timeval tv = { .tv_sec = restoredTime, .tv_usec = 0 };
    settimeofday(&tv, NULL);
  } else {
    struct tm timeinfo;
    timeinfo.tm_year = 126;
    timeinfo.tm_mon = 0;
    timeinfo.tm_mday = 20;
    timeinfo.tm_hour = 4;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeinfo.tm_isdst = 0;
    time_t t = mktime(&timeinfo);
    struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
    settimeofday(&tv, NULL);
  }

  restoreStateAfterWake(showBus72);

  // Check if we woke from sleep via toggle switch
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  if (wakeupCause == ESP_SLEEP_WAKEUP_GPIO) {
    displayDisabled = false;
    rtcDisplayDisabled = false;
    displayOn(display);
  } else {
    displayDisabled = rtcDisplayDisabled;
  }

  offlineMode = true;
  getOfflineDepartures(tram7Departures, tram7Count, bus72Departures, bus72Count);

  if (!displayDisabled) {
    displayDepartures(display, showBus72 ? bus72Departures : tram7Departures,
                     showBus72 ? bus72Count : tram7Count, offlineMode, batteryState);
  }

  if (connectWiFi()) {
    // Use configTzTime which sets TZ and syncs NTP together
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org");

    // Wait for NTP sync - check if time jumps significantly or SNTP reports synced
    time_t timeBeforeSync = time(nullptr);
    int attempts = 0;
    while (attempts < 20) {
      delay(500);
      attempts++;
      time_t currentTime = time(nullptr);
      // If time jumped by more than 10 seconds, NTP has synced
      if (abs(currentTime - timeBeforeSync) > 10) {
        break;
      }
    }

    if (fetchDepartures(tram7Departures, tram7Count, bus72Departures, bus72Count)) {
      offlineMode = false;

      if (!displayDisabled) {
        displayDepartures(display, showBus72 ? bus72Departures : tram7Departures,
                         showBus72 ? bus72Count : tram7Count, offlineMode, batteryState);
      }
    }
  }

  lastUpdate = millis();
  lastToggle = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // Check switch state
  // Switch closed (to GND) = LOW = sleep position
  // Switch open = HIGH (via pull-up) = awake position
  bool switchInSleepPosition = (digitalRead(BUTTON_PIN) == LOW);

  if (switchInSleepPosition && !displayDisabled) {
    displayDisabled = true;
    rtcDisplayDisabled = true;
    saveStateBeforeSleep(showBus72);
    saveTimeToNVS(time(nullptr));
    displayOff(display);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    enterDeepSleep();
    // Device resets on wake - code never reaches here
  }

  if (currentMillis - lastUpdate >= UPDATE_INTERVAL) {
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }

    if (fetchDepartures(tram7Departures, tram7Count, bus72Departures, bus72Count)) {
      offlineMode = false;

      if (!displayDisabled) {
        displayDepartures(display, showBus72 ? bus72Departures : tram7Departures,
                         showBus72 ? bus72Count : tram7Count, offlineMode, batteryState);
      }
      lastToggle = currentMillis;
    } else {
      offlineMode = true;
      getOfflineDepartures(tram7Departures, tram7Count, bus72Departures, bus72Count);

      if (!displayDisabled) {
        displayDepartures(display, showBus72 ? bus72Departures : tram7Departures,
                         showBus72 ? bus72Count : tram7Count, offlineMode, batteryState);
      }
      lastToggle = currentMillis;
    }

    lastUpdate = currentMillis;
  }

  if (currentMillis - lastToggle >= TOGGLE_INTERVAL) {
    showBus72 = !showBus72;

    if (!displayDisabled) {
      displayDepartures(display, showBus72 ? bus72Departures : tram7Departures,
                       showBus72 ? bus72Count : tram7Count, offlineMode, batteryState);
    }
    lastToggle = currentMillis;
  }

  if (currentMillis - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL) {
    updateBatteryState(batteryState);
    lastBatteryUpdate = currentMillis;

    if (!displayDisabled) {
      displayDepartures(display, showBus72 ? bus72Departures : tram7Departures,
                       showBus72 ? bus72Count : tram7Count, offlineMode, batteryState);
    }
  }

  if (currentMillis - lastNVSSave >= NVS_SAVE_INTERVAL) {
    saveTimeToNVS(time(nullptr));
    lastNVSSave = currentMillis;
  }

  // Feed watchdog to prevent resets
  yield();
  delay(100);
}

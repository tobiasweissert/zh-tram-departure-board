#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
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

// RTC Memory Variables (persist across deep sleep)
RTC_DATA_ATTR time_t rtcLastTimestamp = 0;
RTC_DATA_ATTR bool rtcShowBus = true;
RTC_DATA_ATTR bool rtcDisplayDisabled = false;

// Global Variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK,
                         OLED_DC, OLED_RESET, OLED_CS);

Departure tramDepartures[5];
Departure busDepartures[5];
int tramCount = 0;
int busCount = 0;

bool showBus = true;
bool displayDisabled = false;
unsigned long lastToggle = 0;
unsigned long lastUpdate = 0;
unsigned long lastBatteryUpdate = 0;
unsigned long lastNVSSave = 0;
unsigned long lastLoopTime = 0;  // For non-blocking loop timing
bool offlineMode = false;

BatteryState batteryState;

void refreshDisplay() {
  if (!displayDisabled) {
    displayDepartures(display, showBus ? busDepartures : tramDepartures,
                     showBus ? busCount : tramCount, offlineMode, batteryState);
  }
}

// Adaptive polling: increase interval when battery is low
unsigned long getAdaptiveUpdateInterval() {
  if (batteryState.percentage < 20) {
    return UPDATE_INTERVAL * 2;  // Double interval when battery < 20%
  }
  return UPDATE_INTERVAL;
}

void setup() {
  if (!initDisplay(display)) {
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

  restoreStateAfterWake(showBus);

  // Check if we woke from sleep via toggle switch
  esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
  if (wakeupCause == ESP_SLEEP_WAKEUP_GPIO) {
    displayDisabled = false;
    rtcDisplayDisabled = false;
    displayOn(display);
  } else {
    displayDisabled = rtcDisplayDisabled;
  }

  // Reset display cache on boot
  resetDisplayCache();

  offlineMode = true;
  getOfflineDepartures(tramDepartures, tramCount, busDepartures, busCount);

  refreshDisplay();

  // Skip network operations during offline hours (1:00-4:30 AM)
  if (!isOfflineHours() && connectWiFi()) {
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

    if (fetchDepartures(tramDepartures, tramCount, busDepartures, busCount)) {
      offlineMode = false;
      refreshDisplay();
    }
  }

  lastUpdate = millis();
  lastToggle = millis();
  lastLoopTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking loop timing - replace delay(100) with millis check
  if (currentMillis - lastLoopTime < 100) {
    yield();  // Feed watchdog
    return;   // Skip this iteration, not enough time has passed
  }
  lastLoopTime = currentMillis;

  // Check switch state
  // Switch closed (to GND) = LOW = sleep position
  // Switch open = HIGH (via pull-up) = awake position
  bool switchInSleepPosition = (digitalRead(BUTTON_PIN) == LOW);

  if (switchInSleepPosition && !displayDisabled) {
    displayDisabled = true;
    rtcDisplayDisabled = true;
    saveStateBeforeSleep(showBus);
    saveTimeToNVS(time(nullptr));
    displayOff(display);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    enterDeepSleep();
    // Device resets on wake - code never reaches here
  }

  // Use adaptive update interval based on battery level
  unsigned long adaptiveInterval = getAdaptiveUpdateInterval();

  if (currentMillis - lastUpdate >= adaptiveInterval) {
    // Skip API calls during offline hours (1:00-4:30 AM - no service)
    if (isOfflineHours()) {
      offlineMode = true;
      getOfflineDepartures(tramDepartures, tramCount, busDepartures, busCount);
    } else {
      if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
      }

      if (fetchDepartures(tramDepartures, tramCount, busDepartures, busCount)) {
        offlineMode = false;
      } else {
        offlineMode = true;
        getOfflineDepartures(tramDepartures, tramCount, busDepartures, busCount);
      }
    }

    refreshDisplay();
    lastToggle = currentMillis;

    lastUpdate = currentMillis;
  }

  if (currentMillis - lastToggle >= TOGGLE_INTERVAL) {
    showBus = !showBus;
    refreshDisplay();
    lastToggle = currentMillis;
  }

  if (currentMillis - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL) {
    updateBatteryState(batteryState);
    lastBatteryUpdate = currentMillis;

    // Use partial update for battery icon only (more power efficient)
    if (!displayDisabled) {
      updateBatteryIcon(display, batteryState);
    }
  }

  if (currentMillis - lastNVSSave >= NVS_SAVE_INTERVAL) {
    saveTimeToNVS(time(nullptr));
    lastNVSSave = currentMillis;
  }
}

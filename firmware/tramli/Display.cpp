#include "Display.h"
#include "Config.h"
#include "Icons.h"
#include "Utils.h"
#include <string.h>

// Cached state for partial updates
static int cachedBatteryPercentage = -1;
static char cachedTimeStr[6] = "";

// Display positions for partial updates
static const int BATTERY_ICON_X = 104;
static const int BATTERY_ICON_Y = 1;
static const int TIME_X = 100;

const unsigned char* getBatteryIcon(int percentage) {
  if (percentage >= 87) return batteryFullIcon;
  if (percentage >= 62) return battery75Icon;
  if (percentage >= 37) return battery50Icon;
  if (percentage >= 12) return battery25Icon;
  return batteryEmptyIcon;
}

bool initDisplay(Adafruit_SSD1306& display) {
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    return false;
  }
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(DISPLAY_BRIGHTNESS);
  return true;
}

void displayDepartures(Adafruit_SSD1306& display,
                      Departure* currentDepartures,
                      int currentCount,
                      bool offlineMode,
                      const BatteryState& batteryState) {
  display.clearDisplay();
  display.setFont();
  display.setTextSize(1);

  int16_t x1, y1;
  uint16_t w, h;

  display.setCursor(0, 0);
  display.print("Morgental");

  char timeStr[6];
  getCurrentTimeString(timeStr, sizeof(timeStr));
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);

  int timeX = SCREEN_WIDTH - w;
  int batteryX = timeX - 10;
  display.drawBitmap(batteryX, 1, getBatteryIcon(batteryState.percentage), 8, 5, SSD1306_WHITE);

  display.setCursor(timeX, 0);
  display.print(timeStr);

  if (offlineMode) {
    display.setCursor(batteryX - 8, 0);
    display.print("!");
  }

  display.drawLine(0, 9, SCREEN_WIDTH, 9, SSD1306_WHITE);

  if (currentCount == 0) {
    display.setCursor(0, 18);
    display.println("No departures");
  } else {
    unsigned long currentTime = getCurrentTimestamp();

    for (int i = 0; i < currentCount && i < 5; i++) {
      int yPos = 12 + (i * 10);

      char destBuffer[32];
      strncpy(destBuffer, currentDepartures[i].destination, sizeof(destBuffer) - 1);
      destBuffer[sizeof(destBuffer) - 1] = '\0';

      // Replace ", " with " "
      for (int j = 0; destBuffer[j] != '\0'; j++) {
        if (destBuffer[j] == ',' && destBuffer[j+1] == ' ') {
          destBuffer[j] = ' ';
          int k = j + 1;
          while (destBuffer[k] != '\0') {
            destBuffer[k] = destBuffer[k + 1];
            k++;
          }
        }
      }

      if (strlen(destBuffer) > 14) {
        destBuffer[14] = '\0';
      }

      char displayBuffer[48];
      snprintf(displayBuffer, sizeof(displayBuffer), "%s %s",
               currentDepartures[i].number, destBuffer);
      convertUmlauts(displayBuffer, sizeof(displayBuffer));

      display.setCursor(0, yPos);
      display.print(displayBuffer);

      long secondsUntil = currentDepartures[i].departureTimestamp - currentTime;
      int minutesUntil = secondsUntil / 60;
      char minStr[8];

      if (minutesUntil <= 0) {
        display.setCursor(SCREEN_WIDTH - 26, yPos);
        display.print("-");
        display.drawBitmap(SCREEN_WIDTH - 10, yPos, busFrontIcon, 8, 8, SSD1306_WHITE);
      } else if (minutesUntil < 5) {
        snprintf(minStr, sizeof(minStr), "%d", minutesUntil);
        display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(SCREEN_WIDTH - 26, yPos);
        display.print("-");
        display.setCursor(SCREEN_WIDTH - w - 3, yPos);
        display.print(minStr);
        display.drawFastVLine(SCREEN_WIDTH - 2, yPos, 3, SSD1306_WHITE);
      } else if (minutesUntil < 10) {
        snprintf(minStr, sizeof(minStr), "%d", minutesUntil);
        display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
        display.drawBitmap(SCREEN_WIDTH - 26, yPos, runningManIcon, 8, 8, SSD1306_WHITE);
        display.setCursor(SCREEN_WIDTH - w - 3, yPos);
        display.print(minStr);
        display.drawFastVLine(SCREEN_WIDTH - 2, yPos, 3, SSD1306_WHITE);
      } else {
        snprintf(minStr, sizeof(minStr), "%d", minutesUntil);
        display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(SCREEN_WIDTH - w - 3, yPos);
        display.print(minStr);
        display.drawFastVLine(SCREEN_WIDTH - 2, yPos, 3, SSD1306_WHITE);
      }
    }
  }

  display.display();
}

bool updateBatteryIcon(Adafruit_SSD1306& display, const BatteryState& batteryState) {
  const unsigned char* newIcon = getBatteryIcon(batteryState.percentage);
  const unsigned char* oldIcon = getBatteryIcon(cachedBatteryPercentage);

  if (newIcon == oldIcon && cachedBatteryPercentage >= 0) {
    return false;
  }

  cachedBatteryPercentage = batteryState.percentage;
  display.fillRect(BATTERY_ICON_X, BATTERY_ICON_Y, 8, 5, SSD1306_BLACK);
  display.drawBitmap(BATTERY_ICON_X, BATTERY_ICON_Y, newIcon, 8, 5, SSD1306_WHITE);
  display.display();
  return true;
}

bool updateTimeDisplay(Adafruit_SSD1306& display) {
  char newTimeStr[6];
  getCurrentTimeString(newTimeStr, sizeof(newTimeStr));

  if (strcmp(newTimeStr, cachedTimeStr) == 0) {
    return false;
  }

  strncpy(cachedTimeStr, newTimeStr, sizeof(cachedTimeStr));
  display.fillRect(TIME_X, 0, 28, 8, SSD1306_BLACK);
  display.setCursor(TIME_X, 0);
  display.print(newTimeStr);
  display.display();
  return true;
}

void resetDisplayCache() {
  cachedBatteryPercentage = -1;
  cachedTimeStr[0] = '\0';
}

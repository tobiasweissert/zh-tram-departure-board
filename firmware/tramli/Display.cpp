#include "Display.h"
#include "Config.h"
#include "Icons.h"
#include "Utils.h"

bool initDisplay(Adafruit_SSD1306& display) {
  return display.begin(SSD1306_SWITCHCAPVCC);
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

  String timeStr = getCurrentTimeString();
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);

  int timeX = SCREEN_WIDTH - w;

  // Battery icon (8x5 pixels)
  int batteryX = timeX - 10;
  const unsigned char* batteryIcon;
  if (batteryState.percentage >= 87) {
    batteryIcon = batteryFullIcon;
  } else if (batteryState.percentage >= 62) {
    batteryIcon = battery75Icon;
  } else if (batteryState.percentage >= 37) {
    batteryIcon = battery50Icon;
  } else if (batteryState.percentage >= 12) {
    batteryIcon = battery25Icon;
  } else {
    batteryIcon = batteryEmptyIcon;
  }
  display.drawBitmap(batteryX, 1, batteryIcon, 8, 5, SSD1306_WHITE);

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

      // Pre-allocate buffer to reduce String allocations
      char destBuffer[32];
      strncpy(destBuffer, currentDepartures[i].destination.c_str(), 31);
      destBuffer[31] = '\0';

      // Replace ", " with " " manually
      for (int j = 0; destBuffer[j] != '\0'; j++) {
        if (destBuffer[j] == ',' && destBuffer[j+1] == ' ') {
          destBuffer[j] = ' ';
          // Shift rest of string left
          int k = j + 1;
          while (destBuffer[k] != '\0') {
            destBuffer[k] = destBuffer[k + 1];
            k++;
          }
        }
      }

      // Truncate to 14 chars
      if (strlen(destBuffer) > 14) {
        destBuffer[14] = '\0';
      }

      // Build display text: "number dest"
      char displayBuffer[48];
      snprintf(displayBuffer, sizeof(displayBuffer), "%s %s",
               currentDepartures[i].number.c_str(), destBuffer);

      String displayText = convertUmlauts(String(displayBuffer));

      display.setCursor(0, yPos);
      display.print(displayText);

      long secondsUntil = currentDepartures[i].departureTimestamp - currentTime;
      int minutesUntil = secondsUntil / 60;

      if (minutesUntil <= 0) {
        display.setCursor(SCREEN_WIDTH - 26, yPos);
        display.print("-");
        display.drawBitmap(SCREEN_WIDTH - 10, yPos, busFrontIcon, 8, 8, SSD1306_WHITE);
      } else if (minutesUntil < 5) {
        String minStr = String(minutesUntil);
        display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(SCREEN_WIDTH - 26, yPos);
        display.print("-");
        display.setCursor(SCREEN_WIDTH - w - 3, yPos);
        display.print(minStr);
        display.drawFastVLine(SCREEN_WIDTH - 2, yPos, 3, SSD1306_WHITE);
      } else if (minutesUntil < 10) {
        String minStr = String(minutesUntil);
        display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
        display.drawBitmap(SCREEN_WIDTH - 26, yPos, runningManIcon, 8, 8, SSD1306_WHITE);
        display.setCursor(SCREEN_WIDTH - w - 3, yPos);
        display.print(minStr);
        display.drawFastVLine(SCREEN_WIDTH - 2, yPos, 3, SSD1306_WHITE);
      } else {
        String minStr = String(minutesUntil);
        display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
        display.setCursor(SCREEN_WIDTH - w - 3, yPos);
        display.print(minStr);
        display.drawFastVLine(SCREEN_WIDTH - 2, yPos, 3, SSD1306_WHITE);
      }
    }
  }

  display.display();
}

#include "PowerManager.h"
#include "Config.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <esp_chip_info.h>

void displayOff(Adafruit_SSD1306& display) {
  display.clearDisplay();
  display.display();
  display.ssd1306_command(0xAE);
}

void displayOn(Adafruit_SSD1306& display) {
  display.ssd1306_command(0xAF);
}

void enterDeepSleep() {
  gpio_num_t btn = (gpio_num_t)BUTTON_PIN;

  // Hold GPIO state during deep sleep
  gpio_hold_en(btn);

  // Configure EXT1 wakeup - wake on HIGH (switch open)
  esp_sleep_enable_ext1_wakeup_io(1ULL << btn, ESP_EXT1_WAKEUP_ANY_HIGH);

  esp_deep_sleep_start();
}

extern RTC_DATA_ATTR uint32_t bootCount;
extern RTC_DATA_ATTR time_t rtcLastTimestamp;
extern RTC_DATA_ATTR bool rtcShowBus72;

void saveStateBeforeSleep(bool currentShowBus72) {
  bootCount++;
  rtcLastTimestamp = time(nullptr);
  rtcShowBus72 = currentShowBus72;
}

void restoreStateAfterWake(bool& showBus72) {
  showBus72 = rtcShowBus72;
}

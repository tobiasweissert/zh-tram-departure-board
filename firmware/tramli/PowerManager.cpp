#include "PowerManager.h"
#include "Config.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <driver/rtc_io.h>
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

  // Keep RTC peripherals powered during deep sleep for internal pull-ups to work
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

  // Initialize GPIO as RTC IO with pull-up enabled
  rtc_gpio_init(btn);
  rtc_gpio_set_direction(btn, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en(btn);
  rtc_gpio_pulldown_dis(btn);

  // Enable deep sleep GPIO wakeup - wake when GPIO goes HIGH (switch open)
  esp_deep_sleep_enable_gpio_wakeup(1ULL << btn, ESP_GPIO_WAKEUP_GPIO_HIGH);

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

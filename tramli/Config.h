#ifndef CONFIG_H
#define CONFIG_H

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI   18
#define OLED_CLK    19
#define OLED_DC     22
#define OLED_CS     21
#define OLED_RESET   1

#define BATTERY_ADC_PIN  0  // GPIO0 (D0) - explicit GPIO number for ESP32-C6 ADC

#define BUTTON_PIN   2

static const char* WIFI_SSID = "Sunrise_Wi-Fi_F57580";
static const char* WIFI_PASSWORD = "marI7ne_Valid+type";

static const char* API_URL = "http://transport.opendata.ch/v1/stationboard?station=Zurich,Morgental&limit=40&transportations=tram,bus&fields[]=stationboard/number&fields[]=stationboard/to&fields[]=stationboard/stop/departureTimestamp";
const unsigned long UPDATE_INTERVAL = 30000;
const unsigned long TOGGLE_INTERVAL = 10000;
const unsigned long BATTERY_UPDATE_INTERVAL = 5000;

#define NVS_NAMESPACE "tramli"
#define NVS_SAVE_INTERVAL 60000

#endif

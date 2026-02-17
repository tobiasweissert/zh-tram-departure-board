#ifndef CONFIG_H
#define CONFIG_H

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DISPLAY_BRIGHTNESS 0x7F  // 0x00 (dim) to 0xFF (bright)

// SPI Pin Configuration for OLED
#define OLED_MOSI   18
#define OLED_CLK    19
#define OLED_DC     22
#define OLED_CS     21
#define OLED_RESET   1

// Battery monitoring
#define BATTERY_ADC_PIN  0  // GPIO0 (D0) - explicit GPIO number for ESP32-C6 ADC

// Sleep/wake toggle switch
#define BUTTON_PIN   2

// WiFi credentials - EDIT THESE
static const char* WIFI_SSID = "YourNetworkName";
static const char* WIFI_PASSWORD = "YourPassword";

// Station configuration
#define STATION_DISPLAY_NAME "Paradeplatz"
#define STATION_API_NAME "Zurich,Paradeplatz"

// Tram line - the display toggles between tram and bus departures
#define TRAM_LINE_NUMBER "2"
#define TRAM_EXCLUDE_DESTINATION ""  // e.g. "Farbhof" to filter out a direction

// Bus line
#define BUS_LINE_NUMBER "8"
#define BUS_EXCLUDE_DESTINATION ""

// API configuration
// Find your station at https://transport.opendata.ch
static const char* API_URL = "http://transport.opendata.ch/v1/stationboard?station=" STATION_API_NAME "&limit=40&transportations=tram,bus&fields[]=stationboard/number&fields[]=stationboard/to&fields[]=stationboard/stop/departureTimestamp";

// Update intervals (milliseconds)
const unsigned long UPDATE_INTERVAL = 30000;          // Fetch departures every 30s
const unsigned long TOGGLE_INTERVAL = 10000;          // Switch display every 10s
const unsigned long BATTERY_UPDATE_INTERVAL = 5000;   // Update battery every 5s

// NVS storage
#define NVS_NAMESPACE "tramli"
#define NVS_SAVE_INTERVAL 60000  // Save time to NVS every 60s

#endif

# Configuration Guide

All configuration is done in `firmware/tramli/Config.h`. Copy from `Config.example.h` to get started.

## WiFi Settings

```cpp
static const char* WIFI_SSID = "YourNetworkName";
static const char* WIFI_PASSWORD = "YourPassword";
```

## Changing Stations

### 1. Update the API URL

Find your station at [transport.opendata.ch](https://transport.opendata.ch) and update:

```cpp
static const char* API_URL = "http://transport.opendata.ch/v1/stationboard?station=YOUR_STATION&limit=40";
```

Use the exact station name as shown in the API (e.g., "Zurich, Bellevue").

### 2. Modify Line Filtering

Edit `firmware/tramli/NetworkManager.cpp` to filter for your lines:

```cpp
// Current code filters for Tram 7 and Bus 72
// Change these conditions for your lines:
if (dep.number == "72") {
    // Bus 72 logic
}
else if (dep.number == "7" &&
         dep.destination.indexOf("Wollishoferplatz") < 0) {
    // Tram 7 logic - excludes wrong direction
}
```

### 3. Update Offline Schedules

For offline mode fallback:

1. Edit `tools/generate_schedules.py`:
   - Change `STATION_NAME` to your station
   - Change `ROUTE_NUMBERS` to your lines

2. Run the generator:
   ```bash
   cd tools
   python3 generate_schedules.py
   mv ScheduleData.h ../firmware/tramli/data/
   ```

## Pin Configuration

Modify these if using different GPIO pins:

```cpp
// OLED SPI pins
#define OLED_MOSI   18
#define OLED_CLK    19
#define OLED_DC     22
#define OLED_CS     21
#define OLED_RESET   1

// Other pins
#define BUTTON_PIN   2      // Toggle switch for sleep/wake
#define BATTERY_ADC_PIN  0  // Optional battery monitoring
```

## Timing Settings

```cpp
const unsigned long UPDATE_INTERVAL = 30000;          // Fetch departures every 30s
const unsigned long TOGGLE_INTERVAL = 10000;          // Switch display every 10s
const unsigned long BATTERY_UPDATE_INTERVAL = 5000;   // Update battery every 5s
```

## Display Settings

```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
```

These match the standard SSD1306 OLED resolution.

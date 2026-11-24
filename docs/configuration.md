# Configuration Guide

All configuration is done in `firmware/zh_tram_departure_board/Config.h`. Copy from `Config.example.h` to get started.

## WiFi Settings

```cpp
static const char* WIFI_SSID = "YourNetworkName";
static const char* WIFI_PASSWORD = "YourPassword";
```

## Changing Stations

### 1. Update Station and Line Settings

In your `Config.h`, set your station and which tram/bus lines to display. The display toggles between tram and bus departures:

```cpp
// Station configuration
#define STATION_DISPLAY_NAME "Paradeplatz"
#define STATION_API_NAME "Zurich,Paradeplatz"

// Tram line
#define TRAM_LINE_NUMBER "2"
#define TRAM_EXCLUDE_DESTINATION ""  // e.g. "Farbhof" to filter out a direction

// Bus line
#define BUS_LINE_NUMBER "8"
#define BUS_EXCLUDE_DESTINATION ""
```

Find your station name at [transport.opendata.ch](https://transport.opendata.ch). Use the exact station name as shown in the API (e.g., `"Zurich,Bellevue"`).

To exclude a specific direction, set the exclude destination (e.g., `"Paradeplatz"` to filter out departures heading that way).

### 2. Update Offline Schedules

For offline mode fallback:

1. Create `tools/config.local.py` (copy from `config.example.py`):
   ```python
   STATION_NAME = "Paradeplatz"
   STATION_CITY = "Zürich"
   TRAM_ROUTE = "2"
   BUS_ROUTE = "8"
   TRAM_EXCLUDE_DESTINATION = ""
   BUS_EXCLUDE_DESTINATION = ""
   ```

2. Run the generator:
   ```bash
   cd tools
   python3 generate_schedules.py
   ```

This generates `firmware/zh_tram_departure_board/ScheduleData.h` with real departure times from GTFS data.

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

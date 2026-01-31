# Tramli - ESP32-C6 Departure Board

Real-time tram and bus departure display for Zürich Morgental station with offline fallback capability.

## Features

- **Real-time API Mode**: Fetches live departures from transport.opendata.ch every 30 seconds
- **Offline Mode**: Falls back to pre-downloaded schedules when WiFi/API unavailable
- **Smart Display**: Shows Tram 7 and Bus 72 with alternating 10-second display
- **Battery Monitoring**: Displays battery level and charging status (requires voltage divider)
- **Power Management**: Light sleep mode with button wake
  - Long-press button (2 seconds) to enter ultra-low power sleep
  - Press button to wake up instantly
  - Extends battery life significantly
- **Status Icons**:
  - Battery level indicator (5 levels: 0%, 25%, 50%, 75%, 100%)
  - Charging indicator (lightning bolt overlay when charging)
  - WiFi status icon (connected/disconnected)
- **Visual Indicators**:
  - Running man icon for urgent departures (5-9 minutes)
  - Bus icon for departures leaving now
  - "OFFLINE" indicator when using fallback schedules
- **128x64 OLED Display**: Clear, readable departure information

## Hardware

- **Board**: XIAO ESP32-C6 (4MB flash, 512KB RAM)
- **Display**: 128x64 OLED (SSD1306, SPI interface)
- **Button**: Onboard BOOT button (GPIO 9 - no external wiring needed!)
- **Connections**:
  - OLED_MOSI: D10 (GPIO 18)
  - OLED_CLK: D8 (GPIO 19)
  - OLED_DC: D2 (GPIO 2)
  - OLED_CS: D3 (GPIO 21)
  - OLED_RESET: D1 (GPIO 1)

### Battery Monitoring (Optional)

To enable battery level and charging status display, you need to add a voltage divider circuit:

**Required Components**:
- 2x 200kΩ resistors (1/4W or higher)
- LiPo battery (3.7V nominal, 4.2V max)

**Voltage Divider Wiring**:

```
Battery + ──┬─── (to battery pad on ESP32-C6)
            │
           [200kΩ resistor]
            │
            ├──── A0 (GPIO D0)
            │
           [200kΩ resistor]
            │
            └─── GND
```

**Soldering Instructions**:

1. **Locate the battery pads** on the XIAO ESP32-C6:
   - Negative pad: Left side near "D8" silkscreen marking
   - Positive pad: Right side near "D5" silkscreen marking

2. **Solder the voltage divider**:
   - Solder one 200kΩ resistor between Battery+ pad and A0 pin (GPIO D0)
   - Solder second 200kΩ resistor between A0 pin (GPIO D0) and GND pin
   - Keep resistor leads short to minimize noise

3. **Connect battery**:
   - Solder battery negative wire to Battery- pad
   - Solder battery positive wire to Battery+ pad
   - Ensure polarity is correct to avoid damage

**Why this is needed**:
- LiPo battery voltage ranges from 3.2V (empty) to 4.2V (full)
- ESP32-C6 ADC maximum input is 3.3V
- The voltage divider reduces battery voltage by half (4.2V → 2.1V)
- This protects the ADC from overvoltage damage

**Safety Notes**:
- ⚠️ Double-check polarity before connecting battery
- ⚠️ Use a LiPo battery with built-in protection circuit
- ⚠️ Never connect battery backwards (will damage board)
- ⚠️ Ensure resistors are rated for at least 1/4W
- ⚠️ Do not short battery terminals

**Without voltage divider**: The code will still run, but battery monitoring will not work correctly and may damage the ADC if battery voltage exceeds 3.3V.

## Software Requirements

### Arduino IDE Setup

1. Install Arduino IDE 2.x
2. Add ESP32 board support:
   - File > Preferences > Additional Board Manager URLs
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Install ESP32 boards: Tools > Board Manager > "esp32"
4. Select board: XIAO ESP32C6

### Required Libraries

Install via Arduino Library Manager:
- Adafruit GFX Library
- Adafruit SSD1306
- ArduinoJson
- LittleFS (included with ESP32 core)

### LittleFS Upload Plugin

For uploading offline schedules:
1. Download: https://github.com/lorol/arduino-esp32littlefs-plugin
2. Extract to `~/Documents/Arduino/tools/`
3. Restart Arduino IDE

## Setup Instructions

### 1. Configure WiFi

Edit `src/Config.h`:
```cpp
static const char* WIFI_SSID = "YourWiFiSSID";
static const char* WIFI_PASSWORD = "YourPassword";
```

### 2. Generate Offline Schedules (Optional but Recommended)

```bash
# Install Python requirements (if needed)
pip3 install requests

# Run schedule generator from src directory
cd src
python3 generate_schedules.py
```

This creates:
- `data/schedules/tram7.bin` (~8KB)
- `data/schedules/bus72.bin` (~8KB)

### 3. Upload Code

1. Connect ESP32-C6 via USB
2. Arduino IDE: Sketch > Upload

### 4. Upload Offline Schedules (Optional)

1. Arduino IDE: Tools > ESP32 Sketch Data Upload
2. Uploads `data/` folder to ESP32 LittleFS

## Operation Modes

### API Mode (Primary)

- Active when WiFi/API available
- Fetches real-time departures every 30 seconds
- Displays current time in header
- Most accurate departure times

### Offline Mode (Fallback)

- Activates automatically when API unavailable
- Uses pre-downloaded GTFS schedules
- Displays "OFFLINE" in header
- Automatically returns to API mode when connection restored

### Power Management

- **Long press (2 seconds)**: Enter light sleep mode
  - Display turns off
  - WiFi enters sleep mode
  - Ultra-low power consumption
  - Wakes with button press
- **Wake up**: Press button to resume normal operation
  - Time is preserved during sleep using RTC and NVS
  - WiFi reconnects automatically
  - Display shows last known departures immediately

### Mode Indicators

| Display Header | Mode | Data Source |
|----------------|------|-------------|
| `Morgental 14:23` | API Mode | Real-time API |
| `Morgental OFFLINE` | Offline Mode | Local schedules |

## Display Information

### Line Display

Shows 5 departures alternating between:
- **Tram 7** (to Stettbach, excluding Wollishoferplatz)
- **Bus 72** (all directions)

### Departure Format

```
7 Stettbach           12'
72 Morgental Bahnhof   3'
```

- Line number
- Destination (truncated to 14 chars)
- Minutes until departure
- Special icons for urgent departures

## File Structure

```
tramli/
├── README.md                    # This file
└── src/                         # Source code directory
    ├── tramli.ino              # Main Arduino sketch
    ├── Config.h                # Configuration constants
    ├── DataStructures.h        # Data type definitions
    ├── Icons.h                 # Display icon bitmaps
    ├── BatteryMonitor.cpp/h    # Battery monitoring system
    ├── Display.cpp/h           # Display management
    ├── NetworkManager.cpp/h    # WiFi and API handling
    ├── OfflineSchedule.cpp/h   # Offline schedule loader
    ├── ButtonHandler.cpp/h     # Button input handling
    ├── PowerManager.cpp/h      # Power management system
    ├── Utils.cpp/h             # Utility functions
    ├── ScheduleData.h          # Binary schedule format
    └── generate_schedules.py   # GTFS to binary converter
```

## Code Architecture

### Main Components

- **tramli.ino**: Main setup and loop logic, handles state transitions
- **Config.h**: All pin definitions, WiFi credentials, and timing constants
- **Display**: Renders departures, battery status, and icons to OLED
- **NetworkManager**: Handles WiFi connection and API requests
- **BatteryMonitor**: ADC reading and battery state calculation
- **OfflineSchedule**: Loads and queries pre-downloaded schedules
- **ButtonHandler**: Debounced button input with long-press detection
- **PowerManager**: Light sleep mode with wake functionality
- **Utils**: Time formatting and helper functions

### Data Flow

1. **Setup Phase**:
   - Initialize display and peripherals
   - Restore time from RTC/NVS
   - Load offline schedules
   - Connect WiFi and sync time

2. **Runtime Loop**:
   - Check button for power management
   - Fetch API data every 30 seconds
   - Toggle display between Tram 7 and Bus 72 every 10 seconds
   - Update battery status every 5 seconds
   - Save time to NVS every 60 seconds

3. **Sleep/Wake Cycle**:
   - Button long-press triggers sleep
   - Save time to RTC and NVS
   - Enter light sleep with timer wake
   - Button press exits sleep
   - Restore time and resume operation

## Memory Usage

### Flash (4MB total)
- Program code: ~300KB
- LittleFS partition: 512KB
  - Schedule files: ~16KB
  - Free: ~496KB
- OTA partition: 1.5MB
- Free: ~1.5MB

### RAM (512KB total)
- Current usage: ~50KB
- Schedules in RAM: ~20KB
- Display buffer: ~1KB
- WiFi stack: ~40KB
- Free: ~393KB

## Schedule Updates

Update offline schedules when:
- Timetables change (typically twice yearly)
- Route modifications occur
- Before holidays with special schedules

Process:
1. Run `python3 generate_schedules.py` in src directory
2. Upload data folder to ESP32 using LittleFS tool
3. Reset device

## Troubleshooting

### No WiFi Connection
- Check SSID and password in `src/Config.h`
- Verify WiFi network is 2.4GHz (ESP32-C6 doesn't support 5GHz)
- If offline schedules present, device will automatically switch to offline mode

### No Offline Schedules
- Run `generate_schedules.py` to create schedule files
- Upload data folder using LittleFS upload tool
- Check serial monitor for "schedules loaded" message

### Display Issues
- Verify SPI pin connections in `src/Config.h`
- Check display address (SPI, not I2C)
- Test with Adafruit SSD1306 examples

### Time Drift in Offline Mode
- Normal - RTC will drift without NTP sync
- Automatically corrects when WiFi reconnects
- Time is saved to NVS every 60 seconds to persist across reboots

### Wrong Departures in Offline Mode
- Update schedule files with latest GTFS data
- Verify correct station name in `generate_schedules.py`
- Check schedule file dates and validity periods

### Battery Reading Issues
- Verify voltage divider is properly soldered
- Check resistor values (both should be 200kΩ)
- Measure voltage at A0 pin (should be ~2.1V with full battery)
- Battery voltage should never exceed 3.3V at ADC pin

### Sleep Mode Issues
- If device doesn't wake, check button connection
- Ensure button is onboard BOOT button (GPIO 9)
- Time is preserved in RTC memory during sleep
- Time is also saved to NVS (flash) to survive reboots

## Data Sources

- **Real-time API**: https://transport.opendata.ch
- **GTFS Schedules**: https://opentransportdata.swiss

## License

MIT License - Free for personal and commercial use

## Contributing

Contributions welcome! Areas for improvement:
- Additional stations
- More transport lines
- Weather display
- OTA updates for schedules
- Web configuration interface
- Deep sleep mode optimization

## Acknowledgments

- Swiss Open Transport Data for API and GTFS feeds
- Adafruit for excellent display libraries
- Arduino and ESP32 communities

# Programming Guide

## Prerequisites

### Arduino IDE Setup

1. Add ESP32 board URL in **File > Preferences > Additional Board Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

2. Install ESP32 boards:
   - Go to **Tools > Board > Boards Manager**
   - Search for "esp32" and install

3. Select board: **Tools > Board > ESP32C6 Dev Module**

4. Install required libraries via **Sketch > Include Library > Manage Libraries**:
   - Adafruit GFX Library
   - Adafruit SSD1306
   - ArduinoJson

### Arduino CLI Setup

```bash
# Install ESP32 core
arduino-cli core install esp32:esp32

# Install libraries
arduino-cli lib install "Adafruit GFX Library" "Adafruit SSD1306" "ArduinoJson"
```

## Configuration

Before building, copy and configure the settings file:

```bash
cd firmware/tramli
cp Config.example.h Config.h
```

Edit `Config.h` with your WiFi credentials:

```cpp
static const char* WIFI_SSID = "YourNetworkName";
static const char* WIFI_PASSWORD = "YourPassword";
```

See [configuration.md](configuration.md) for detailed settings.

## Building

### Arduino IDE

1. Open `firmware/tramli/tramli.ino`
2. Select **ESP32C6 Dev Module** as the board
3. Click **Verify** to compile

### Arduino CLI

```bash
arduino-cli compile --fqbn esp32:esp32:esp32c6 firmware/tramli
```

## Uploading

### Arduino IDE

1. Connect the ESP32-C6 via USB
2. Select the correct port under **Tools > Port**
3. Click **Upload**

### Arduino CLI

```bash
# Find your port (usually /dev/cu.usbmodem* on macOS)
arduino-cli board list

# Upload
arduino-cli upload -p /dev/cu.usbmodem* --fqbn esp32:esp32:esp32c6 firmware/tramli
```

## Troubleshooting

### Compilation Errors

- Ensure all libraries are installed
- Check that `Config.h` exists (copied from `Config.example.h`)
- Verify ESP32 board package is installed

### Upload Fails

- Try holding the BOOT button while uploading
- Check USB cable supports data transfer
- Ensure correct port is selected

### Display Not Working

- Verify SPI wiring matches pin configuration
- Check display receives 3.3V power
- Ensure OLED module is SPI type (not I2C)

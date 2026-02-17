# Assembly Guide

## Parts List

| Qty | Component | Notes |
|-----|-----------|-------|
| 1 | Seeed Studio XIAO ESP32-C6 | Main microcontroller |
| 1 | 128x64 OLED SSD1306 (SPI) | Display module |
| 1 | SPST toggle switch | Power on/off |
| 1 | 3.7V LiPo battery | Optional, any capacity |
| 2 | 200k resistors | Optional, for battery monitoring |
| - | Hookup wire | For connections |

## Assembly Steps

### 1. Prepare the ESP32-C6

- Solder header pins if needed
- The XIAO ESP32-C6 has a built-in LiPo charger

### 2. Connect the Display

Wire the OLED to the ESP32-C6 according to the [wiring guide](../hardware/wiring.md):

- MOSI -> GPIO 18
- CLK -> GPIO 19
- DC -> GPIO 22
- CS -> GPIO 21
- RST -> GPIO 1
- VCC -> 3.3V
- GND -> GND

### 3. Install Power Switch

- Connect one terminal to GPIO 2
- Connect other terminal to GND
- The internal pull-up resistor is enabled in software

### 4. Battery Setup (Optional)

If using battery power:

1. Connect LiPo to the battery pads on the ESP32-C6
2. Add voltage divider for monitoring:
   - Two 200k resistors in series from battery+ to GND
   - Connect midpoint to GPIO 0

### 5. Final Assembly

- Mount components in your enclosure
- Ensure switch is accessible
- Allow visibility of the display

## Testing

1. Power on the device
2. The display should show your station name and attempt WiFi connection
3. Toggle switch to test sleep/wake functionality

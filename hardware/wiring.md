# Wiring Guide

## Components

| Component | Model | Notes |
|-----------|-------|-------|
| Microcontroller | Seeed Studio XIAO ESP32-C6 | WiFi 6, low power |
| Display | 128x64 OLED SSD1306 | SPI interface |
| Power switch | SPST toggle switch | Connected to GPIO 2 |
| Battery | 3.7V LiPo | Optional, with voltage divider |

## OLED Display Wiring

```
ESP32-C6          OLED SSD1306
---------         ------------
GPIO 18 (MOSI) -> SDA/MOSI
GPIO 19 (CLK)  -> SCL/CLK
GPIO 22 (DC)   -> DC
GPIO 21 (CS)   -> CS
GPIO 1  (RST)  -> RES/RST
3.3V           -> VCC
GND            -> GND
```

## Power Switch Wiring

```
ESP32-C6          Toggle Switch
---------         -------------
GPIO 2         -> One terminal
GND            -> Other terminal

Switch open  = Device awake (GPIO reads HIGH via internal pull-up)
Switch closed = Device asleep (GPIO reads LOW)
```

## Battery Monitoring (Optional)

To display battery level, add a voltage divider between battery+ and GPIO 0:

```
Battery+ ---+--- [200k] ---+--- [200k] --- GND
            |               |
            |            GPIO 0 (ADC)
            +--- Battery pad on ESP32-C6
```

This halves the voltage so the 4.2V max battery voltage becomes 2.1V, safe for the 3.3V ADC.

## Pin Summary

| GPIO | Function |
|------|----------|
| 0 | Battery ADC (optional) |
| 1 | OLED Reset |
| 2 | Power switch |
| 18 | OLED MOSI |
| 19 | OLED CLK |
| 21 | OLED CS |
| 22 | OLED DC |

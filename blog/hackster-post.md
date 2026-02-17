# Tramli: A Tiny Battery-Powered Tram Departure Display

**Because Swiss trains are on time. I should be too.**

## The Problem

I love the punctuality and reliability of Swiss public transport. So much so that I'm really not used to waiting. That's why I made this little tram departure display. It fetches real-time data from the Swiss open public transport API and shows exactly when the next tram arrives, so I know precisely when to leave the house. Minimum time standing in the cold. I love that this data is publicly available. Open APIs that anyone can build on. Apparently even me. At least something useful they spent my tax money on.

The scenario is always the same: I'm making coffee, my phone is across the room, and by the time I check the transit app the tram has already left. What I wanted was a glanceable display sitting on the kitchen counter. No unlocking, no app switching. Just look up and know. Yes, I could use my phone. But building something sounded more fun. The result: a small battery-powered OLED display that shows live departures and even works offline. Small enough for a countertop, runs for hours on a LiPo cell.

## Things Used

| Component | Notes |
|-----------|-------|
| Seeed Studio XIAO ESP32-C6 | WiFi 6, built-in LiPo charger, tiny footprint |
| 128x64 SSD1306 OLED (SPI) | SPI chosen over I2C for speed |
| SPST toggle switch | Sleep/wake control via GPIO 2 |
| 3.7V LiPo battery | Any capacity works |
| 2x 200k ohm resistors | Voltage divider for battery ADC |

That's it. Five components plus some wire.

## The Build: Hardware

The brain is a Seeed Studio XIAO ESP32-C6 -- a tiny board with WiFi 6, a built-in LiPo charging circuit, and enough GPIOs for an SPI display. Here's how everything connects.

The **OLED display** uses SPI with five signal lines:

```
ESP32-C6          OLED SSD1306
GPIO 18 (MOSI) -> SDA/MOSI
GPIO 19 (CLK)  -> SCL/CLK
GPIO 22 (DC)   -> DC
GPIO 21 (CS)   -> CS
GPIO 1  (RST)  -> RES/RST
3.3V           -> VCC
GND            -> GND
```

The **voltage divider** for battery monitoring is two 200k ohm resistors in series between battery positive and ground, with GPIO 0 tapped at the midpoint. LiPo cells peak at 4.2V, but the ESP32-C6's ADC maxes out at 3.3V. The divider halves the voltage so 4.2V becomes a safe 2.1V. The firmware multiplies back by two and maps 3.2V-4.2V to 0-100%.

The **toggle switch** is the most elegant part of the wiring. One terminal goes to GPIO 2, the other to GND. With the internal pull-up resistor enabled, the pin reads HIGH when the switch is open (device awake) and LOW when closed (device asleep). The same GPIO 2 doubles as the deep sleep wakeup source -- so one switch handles both "go to sleep" and "wake up" with zero extra components.

And because the XIAO has a built-in LiPo charger, USB-C charges the battery directly. No external charging circuit needed.

## The Enclosure

The enclosure was designed in Fusion 360 with three constraints: a window for the OLED, a slot for the toggle switch, and USB-C access for charging. The goal was something clean enough to leave on a kitchen counter -- not a "maker aesthetic" bundle of wires and hot glue.

Both the Fusion 360 source file (`.f3d`) and a ready-to-print `.stl` are included in the repo under `enclosure/`. I printed mine in PLA with standard settings. The display friction-fits into the window, and the whole thing snaps together without screws.

It's a simple design, but that's the point. It should look like a little gadget, not a prototype.

## Getting Real-Time Data

The API is `transport.opendata.ch` -- a free, open API for Swiss public transport. No API key, no registration, no rate limits to worry about. One GET request returns the next departures from any stop in Switzerland:

```
http://transport.opendata.ch/v1/stationboard
  ?station=Zurich,Paradeplatz
  &limit=40
  &transportations=tram,bus
```

The firmware fetches this every 30 seconds (or every 60 seconds when battery drops below 20%, to stretch runtime). JSON parsing pulls out the line number, destination, and departure timestamp for each entry.

The 128x64 OLED alternates between showing tram and bus departures every 10 seconds. Each screen shows the station name, a battery icon, the current time, and up to five upcoming departures.

The fun part is the **urgency-based departure icons**. How much time you have changes what you see:

```cpp
if (minutesUntil <= 0) {
  // Bus-front icon -- it's leaving NOW
  display.print("-");
  display.drawBitmap(..., busFrontIcon, ...);
} else if (minutesUntil < 5) {
  // Dash + minutes + tick mark -- hurry!
  display.print("-");
  display.print(minStr);
} else if (minutesUntil < 10) {
  // Running man icon + minutes -- time to move
  display.drawBitmap(..., runningManIcon, ...);
  display.print(minStr);
} else {
  // Just the number -- you're fine
  display.print(minStr);
}
```

10+ minutes? Just a number. 5-9 minutes? A running man icon appears -- time to get moving. Under 5 minutes? A dash and urgency marker. Zero minutes? A little bus-front icon -- it's at the stop right now.

One important detail: **direction filtering**. Most stops serve both directions, but you probably only care about one. The firmware lets you configure an exclude destination and filters these out with a simple `strstr()` check on the destination name.

When WiFi is down, a `!` indicator appears next to the battery icon so you know you're looking at cached or offline data.

## The Offline Fallback

This is probably the most over-engineered part, and honestly my favorite.

The problem: WiFi can fail. Router reboots, first boot in a new location, flaky connections. For a kitchen counter display, reliability matters. If I glance at it, I want to see departure times, not an error screen.

The solution: embed the **complete GTFS timetable** directly into flash memory. Thousands of schedule entries for your configured lines.

GTFS (General Transit Feed Specification) is the open standard that transit agencies worldwide publish their schedules in. Switzerland makes theirs freely available via opentransportdata.swiss. The key insight is that this data is static and predictable -- trams run on fixed schedules, and the schedule data compresses nicely into a compact C struct:

```cpp
struct ScheduleTime {
  unsigned char hour;      // 0-23
  unsigned char minute;    // 0-59
  unsigned char dayMask;   // Bit mask: 0=Mon, 1=Tue, ..., 6=Sun
  const char* destination;
};
```

Each entry is just three bytes plus a string pointer. The `dayMask` is where it gets elegant -- a single byte encodes which days of the week a departure runs using a bitmask. `0x1F` means weekdays (Monday through Friday), `0x60` means weekends, and `0x7F` means every day. One byte, seven days.

Here's what a few entries look like in practice:

```cpp
const ScheduleTime tramSchedule[] = {
  { 4, 50, 0x1F, "Stettbach, Bahnhof"},  // Weekdays 04:50
  { 4, 50, 0x20, "Bahnhofstrasse/HB"},   // Saturday 04:50
  { 4, 50, 0x40, "Bahnhofstrasse/HB"},   // Sunday 04:50
  { 4, 57, 0x0F, "Bahnhofstrasse/HB"},   // Mon-Thu 04:57
  // ... hundreds more entries
};
```

The Python tool (`tools/generate_schedules.py`) handles all of this automatically. It downloads the official GTFS zip from the Swiss open transport data portal, parses the CSV files for your configured stop and routes, deduplicates entries, and emits a ready-to-compile C header file. No manual data entry.

The firmware also knows that between 1:00 AM and 4:30 AM there's no service at all, so it skips API calls entirely during those hours.

I'll be the first to acknowledge this is over-engineered. Most departure displays are just API wrappers. Embedding thousands of schedule entries in flash for a device that's almost always on WiFi is peak maker energy. But the first time the router rebooted and the display seamlessly switched to the offline timetable without missing a beat? Worth it.

## Power Management

A battery-powered display that's always fetching from the network needs to be thoughtful about power.

**Deep sleep** is triggered by the toggle switch. When GPIO 2 reads LOW (switch flipped to the off position), the firmware saves its state to RTC memory, powers off the OLED with the `0xAE` command, disconnects WiFi, and calls `esp_deep_sleep_start()`. The ESP32 draws microamps in this state. Flipping the switch back opens the circuit, GPIO 2 goes HIGH via the internal pull-up, and the chip wakes on the GPIO interrupt.

**Time persistence** was a subtle problem. The ESP32's clock resets on deep sleep. If the device wakes up without WiFi, it has no idea what time it is -- which means the offline schedule is useless. The fix: save the Unix timestamp to NVS flash every 60 seconds, and also to RTC memory right before sleeping. On wake, the firmware restores the timestamp from RTC memory first (faster), falling back to NVS if needed. The clock will drift by however long you slept, but it's close enough for the offline schedule to work immediately, before NTP has a chance to sync.

**NTP sync detection** uses a neat trick. Rather than polling the SNTP status flag (which can be unreliable), the firmware just reads `time()` before and after the sync attempt and checks if the value jumped by more than 10 seconds. Simple and reliable.

**WiFi power saving** is enabled with `WIFI_PS_MAX_MODEM` after connecting. The radio sleeps between DTIM beacon intervals, saving significant current while maintaining the connection.

**Partial display updates** avoid redrawing the entire screen on every cycle. The battery icon only redraws when the percentage crosses a tier threshold, and the clock only redraws when the minute changes. Fewer SPI transactions, less power.

**Battery monitoring** reads GPIO 0 through the voltage divider, averages four ADC samples to reduce noise, multiplies by two (compensating for the divider), and does a linear interpolation between 3.2V (empty) and 4.2V (full).

## Small Details That Matter

A few things I spent more time on than expected:

**UTF-8 umlaut handling.** The transport API returns station names in German, encoded as UTF-8. "Zurich" is actually "Z&uuml;rich" -- the u-umlaut is a two-byte sequence (`0xC3 0xBC`). But the SSD1306's built-in font uses the CP437 code page, where &uuml; is character 129. The `convertUmlauts()` function does byte-level translation from UTF-8 to CP437. Without it, every German station name renders as garbage characters.

**Destination name truncation.** Station names like "Stettbach, Bahnhof" waste precious screen space with the qualifier after the comma. The firmware strips everything after `, ` to show just "Stettbach" -- enough to know which direction.

**Hand-crafted pixel art.** The icons in `Icons.h` are 8x8 pixel bitmaps defined as byte arrays -- a running man, a bus front-view, and five battery level indicators. Each one was drawn bit by bit. There's a particular satisfaction in getting a recognizable running figure into 64 pixels.

**The layout.** 128x64 pixels doesn't sound like much, but it's enough for a header bar (station name, battery icon, clock), a separator line, and five departure rows at 10 pixels each. Every pixel is accounted for.

## Adapt It for Your City

If you're in Switzerland, adapting Tramli for your stop is straightforward. Change the `API_URL` in `Config.h` to your station name (find it at transport.opendata.ch), update the line number filtering in `NetworkManager.cpp`, and re-run `generate_schedules.py` with your stop and route numbers.

Outside Switzerland, the same architecture works with other open transit APIs. Germany (DELFI), the UK (Transport API), the Netherlands (OVapi) -- many countries publish GTFS feeds or have similar real-time APIs. You'd need to adapt the API parsing, but the firmware structure -- live data with offline fallback, power management, urgency display -- is reusable.

The whole project is MIT licensed and open source.

## Wrapping Up

Tramli sits on my kitchen counter now. I glance at it while making coffee, see that the next tram is in eight minutes, and know exactly when to put on my shoes. No phone, no app, no unlocking anything. Just a number on a tiny screen.

What I enjoy most about this project is how everything fits together: the hardware, the firmware, the 3D-printed case, the offline fallback, the little pixel art icons. It's a complete thing. Not a breadboard prototype, but something that actually lives in my kitchen and gets used every morning.

The full source code, 3D print files, and wiring guide are on GitHub: [github.com/tramli](https://github.com/yourusername/tramli). MIT license. Build it, remix it, adapt it for your city. If you do, I'd love to see it.

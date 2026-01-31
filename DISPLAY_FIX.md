# Display Turn-Off Fix

## Problem Identified

The display was turning back on after being put to sleep due to two issues:

1. **PowerManager.cpp:30** - The `displayOn(display)` function was **always called** when exiting light sleep, even though the intention was to keep the display off.

2. **tramli.ino** - After waking from sleep, `displayDepartures()` was being called throughout the main loop, which would refresh the display buffer even though the display hardware was supposed to be off.

## Root Cause Analysis

When you long-pressed the button:
- `displayOff()` was called (hardware command 0xAE sent) ✓
- Device entered `enterLightSleep()` ✓
- Device woke every 5 seconds via timer to check for button press
- When button was pressed to exit sleep, `displayOn()` was **unconditionally called**
- The main loop continued calling `displayDepartures()`, which would render to the display

## Solution Implemented

### 1. Removed Automatic Display Turn-On (PowerManager.cpp)

**File:** `src/PowerManager.cpp`

**Change:** Removed the `displayOn(display)` call from line 30 in `enterLightSleep()` function.

```cpp
void enterLightSleep(Adafruit_SSD1306& display) {
  while (true) {
    esp_sleep_enable_timer_wakeup(SLEEP_WAKEUP_INTERVAL_US);
    esp_light_sleep_start();

    if (digitalRead(BUTTON_PIN) == LOW) {
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
      delay(50);
      break;
    }
  }

  // REMOVED: displayOn(display);
  // Display stays OFF to save power - no automatic wake
}
```

### 2. Added Persistent Display Disabled Flag (tramli.ino)

**File:** `src/tramli.ino`

**Changes:**

1. Added RTC memory variable to persist across sleep cycles:
   ```cpp
   RTC_DATA_ATTR bool rtcDisplayDisabled = false;
   ```

2. Added runtime flag:
   ```cpp
   bool displayDisabled = false;
   ```

3. Set the flags when entering sleep (line 126-127):
   ```cpp
   displayDisabled = true;
   rtcDisplayDisabled = true;
   ```

4. Restore the flag after waking (line 88):
   ```cpp
   displayDisabled = rtcDisplayDisabled;
   ```

5. Added checks before **every** `displayDepartures()` call throughout the code:
   ```cpp
   if (!displayDisabled) {
     displayDepartures(...);
   }
   ```

## How It Works Now

1. **Initial startup**: Display works normally (displayDisabled = false)

2. **Long-press button**:
   - Sets `displayDisabled = true`
   - Sets `rtcDisplayDisabled = true` (persists in RTC memory)
   - Calls `displayOff(display)` (hardware command 0xAE)
   - Enters light sleep

3. **During sleep**:
   - Display hardware is OFF (0xAE command)
   - Device wakes every 5 seconds to check button

4. **After button press to exit sleep**:
   - `displayOn()` is NO LONGER called
   - Display hardware remains OFF
   - Flag `displayDisabled` is restored from RTC memory
   - Main loop skips all `displayDepartures()` calls

5. **Main loop continues**:
   - WiFi still fetches data
   - Battery monitoring continues
   - Time is synced
   - **Display stays permanently OFF**

## Benefits

- **No manual intervention needed** - Once you long-press the button, the display stays off forever (until you reset/reboot the device)
- **Survives sleep/wake cycles** - The RTC memory flag persists across light sleep
- **Saves maximum power** - Display hardware is OFF (0xAE), no buffer updates, no display refresh
- **All background tasks continue** - WiFi, time sync, data fetching still work normally

## Usage

1. Upload the modified code to your ESP32-C6
2. Device boots normally with display ON
3. Long-press the button for 2 seconds
4. Display turns OFF and stays OFF permanently
5. Device continues to run in the background (WiFi, data fetching, etc.)
6. Press button again to exit sleep - display remains OFF
7. To turn display back ON, you must reset/reboot the device

## Technical Details

- **Display OFF command**: `0xAE` (SSD1306 hardware disable)
- **RTC Memory**: Persists across light sleep but NOT deep sleep or power loss
- **Sleep interval**: 5 seconds (SLEEP_WAKEUP_INTERVAL_US)
- **Display initialization**: Only happens once during setup()

## Future Improvements (Optional)

If you want to toggle the display on/off without rebooting:

1. Use button short-press to toggle `displayDisabled` flag
2. Call `displayOn()` or `displayOff()` based on toggle state
3. Add visual feedback (e.g., blink before turning off)

But for your current need (permanent OFF), this solution is perfect!

## Files Modified

- `src/PowerManager.cpp` - Removed automatic display turn-on
- `src/tramli.ino` - Added display disabled flag and checks

## Compilation

Compile and upload as normal:
```bash
Arduino IDE > Sketch > Upload
```

No additional libraries or dependencies needed.

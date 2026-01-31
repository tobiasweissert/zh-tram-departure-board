# Final Button Behavior - Simple and Intuitive

## Button Controls

### ⏰ Long Press (≥ 2 seconds)
**Turns OFF display and enters sleep mode**

- Display turns OFF
- Device enters low-power sleep mode
- WiFi goes to sleep
- Data fetching stops
- Ultra-low power consumption

### ✨ Short Press (< 2 seconds)
**Turns ON display and wakes from sleep**

- If device is in sleep mode → Wakes up AND turns display ON
- If device is awake with display OFF → Turns display ON
- Display shows current departure times

## Usage Scenarios

### Scenario 1: Turn Display Off for Battery Saving
1. Device is running normally with display ON
2. **Long press** button (hold 2+ seconds)
3. Display turns OFF
4. Device enters sleep mode
5. Minimal power consumption

### Scenario 2: Check Time While Sleeping
1. Device is in sleep mode (display OFF)
2. **Press** button (any length will work)
3. Device wakes from sleep
4. Display turns ON automatically
5. Shows current departure times

### Scenario 3: Turn Display On Without Sleep
1. Device is awake but display is OFF
2. **Short press** button
3. Display turns ON
4. Shows current departure times

### Scenario 4: Put Back to Sleep
1. Display is currently ON
2. **Long press** button (hold 2+ seconds)
3. Display turns OFF
4. Device enters sleep mode again

## Comparison: Old vs New Behavior

| Action | Old Behavior | New Behavior |
|--------|--------------|--------------|
| Long press | Display OFF forever | Display OFF + sleep mode |
| Wake from sleep | Display stays OFF | Display turns ON automatically ✅ |
| Short press when awake | N/A | Display turns ON ✅ |
| Reset to turn ON | Required ❌ | Not needed ✅ |

## Technical Details

### Wake from Sleep Logic

When you press the button to wake from sleep:
1. Button press detected in sleep loop
2. Device exits sleep mode
3. `enterLightSleep()` returns `true` (woken by button)
4. Main code automatically turns display ON
5. Fetches latest departure data
6. Updates display immediately

### Code Flow

**Long Press:**
```cpp
BUTTON_LONG_PRESS detected
  → displayDisabled = true
  → displayOff()
  → enterLightSleep()
  → Wait for button press...
```

**Wake from Sleep:**
```cpp
Button pressed in sleep
  → Exit sleep loop
  → Return true to main code
  → displayDisabled = false
  → displayOn()
  → displayDepartures()  // Show latest data
```

**Short Press (while awake):**
```cpp
BUTTON_SHORT_PRESS detected
  → displayDisabled = false
  → displayOn()
  → displayDepartures()  // Show latest data
```

## Serial Monitor Output

When monitoring via serial (115200 baud):

```
[BUTTON] Long press - turning display OFF and entering sleep
[BUTTON] Woke from sleep with button press - turning display ON
[BUTTON] Short press - turning display ON
```

## Benefits of This Design

✅ **Simple**: One button does everything
✅ **Intuitive**: Press to turn ON, long press to turn OFF
✅ **No reset needed**: Display always accessible via button
✅ **Battery efficient**: Sleep mode saves power
✅ **Immediate feedback**: Display turns ON right when you wake it

## Memory Leak Fixes Included

This deployment also includes all the memory leak fixes:

✅ Fixed `convertUmlauts()` String fragmentation
✅ Changed to `StaticJsonDocument` (no heap allocation)
✅ Optimized Display.cpp String operations
✅ Added heap monitoring (logs every 60 seconds)
✅ Improved WiFi connection handling
✅ Added Serial logging for debugging

## Expected Behavior

**Normal Operation:**
- Device boots → Display ON
- Long press → Display OFF, sleep mode
- Press button → Wake up, display ON
- Device runs indefinitely without reboots

**Serial Output Every 60 Seconds:**
```
[HEAP] Free: 280000, Min free: 275000
```

If free heap stays around 280KB-285KB, the memory leaks are fixed! ✅

## Power Consumption Estimate

- **Display ON + WiFi fetching**: ~80-100mA
- **Display OFF + WiFi fetching**: ~60-80mA
- **Sleep mode (display OFF + WiFi sleep)**: ~5-10mA

With a typical 400mAh LiPo battery:
- Display ON constantly: ~4-5 hours
- Display OFF, wake occasionally: ~8-12 hours
- Sleep mode most of time: ~40-80 hours

## Troubleshooting

### Display doesn't turn ON when pressing button
- Check serial output: should see "[BUTTON] Woke from sleep with button press"
- Verify button is on GPIO 9
- Try resetting device

### Device immediately wakes from sleep
- Timer wake-up happens every 5 seconds (normal)
- Device only exits sleep when button is pressed
- Check serial for "Woke from sleep via timer" (shouldn't appear)

### Display turns ON but shows no data
- WiFi may not be connected
- Check serial for "[WiFi] Connected"
- Wait 30 seconds for first API fetch

## Summary

The button behavior is now simple and intuitive:

- **Long press (2+ sec)** = Turn OFF + sleep
- **Short press (any)** = Turn ON + wake

No more resets needed! The display is always accessible via a button press.

Combined with the memory leak fixes, your tramli departure board should run reliably for days without issues! 🎉

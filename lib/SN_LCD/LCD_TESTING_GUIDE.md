# LCD System Testing Guide

## Pre-Test Hardware Checklist

### LCD Hardware Setup
- [ ] LCD 20x4 with I2C backpack connected
- [ ] I2C SDA connected to ESP32 SDA (GPIO 21)
- [ ] I2C SCL connected to ESP32 SCL (GPIO 22)
- [ ] LCD powered with 5V (not 3.3V!)
- [ ] Ground connected
- [ ] I2C pull-up resistors present (4.7kΩ typical, often on backpack)

### Encoder Hardware Setup
- [ ] Rotary encoder CLK → GPIO 32
- [ ] Rotary encoder DT → GPIO 33
- [ ] Rotary encoder SW → GPIO 23
- [ ] Encoder common pin → GND
- [ ] External 10kΩ pull-up resistors to 3.3V on CLK/DT/SW

---

## Test 1: LCD Initialization (Power-On)

### Procedure
1. Upload firmware to CTU ESP32
2. Open Serial Monitor (115200 baud)
3. Press RESET button on ESP32

### Expected Serial Output
```
[SN_LCD_Init] Scanning for LCD at address 0x27
[SN_LCD_Init] LCD found at address 0x27
```

### Expected LCD Display (First 1 second)
```
     ROVER XR-4
   Initializing...
```

### Expected LCD Display (Next 1 second)
```
  ESP-NOW Connected
      Ready!
```
OR (if ESP-NOW not ready yet)
```
  ESP-NOW Starting
    Please wait...
```

### Expected LCD Display (After 2 seconds)
```
=== ROVER XR-4 ===
State: INIT  
NORMAL  DISARM  
Link:OK OBC:0dB  1/5
```

### ✅ Pass Criteria
- LCD backlight turns on
- Text is readable (not garbled)
- Splash screen displays for 1-2 seconds
- Automatically switches to STATUS page
- Page indicator shows "1/5"

### ❌ Failure Troubleshooting
| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| No backlight | No 5V power | Check power connection |
| Backlight but no text | Contrast too low | Adjust potentiometer on I2C backpack |
| "LCD not found" in serial | I2C wiring issue | Check SDA/SCL connections |
| Garbled characters | I2C noise | Add/check pull-up resistors |

---

## Test 2: Encoder Navigation

### Procedure
1. Wait for STATUS page to appear
2. Rotate encoder **clockwise** slowly (1 click at a time)
3. Observe page changes
4. Continue rotating CW through all pages
5. Rotate encoder **counter-clockwise**
6. Observe pages reverse

### Expected Behavior: Clockwise Rotation
```
Click 1 → Page 2/5 (TELEMETRY)
Click 2 → Page 3/5 (GPS)
Click 3 → Page 4/5 (SENSORS)
Click 4 → Page 5/5 (CONTROL)
Click 5 → Page 1/5 (STATUS) ← Wraps around
```

### Expected Behavior: Counter-Clockwise Rotation
```
Click 1 → Page 5/5 (CONTROL)
Click 2 → Page 4/5 (SENSORS)
Click 3 → Page 3/5 (GPS)
Click 4 → Page 2/5 (TELEMETRY)
Click 5 → Page 1/5 (STATUS) ← Wraps around
```

### Expected Serial Output (per page change)
```
[SN_LCD] Navigated to page 1
[SN_LCD] Navigated to page 2
[SN_LCD] Navigated to page 3
[SN_LCD] Navigated to page 4
[SN_LCD] Navigated to page 0
```

### ✅ Pass Criteria
- Each encoder click changes page
- Page indicator updates correctly
- Pages cycle in correct order
- Wraps around at both ends
- No skipped pages
- No jitter/bouncing

### ❌ Failure Troubleshooting
| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| No page changes | Encoder not wired | Check GPIO 32/33 connections |
| Pages skip randomly | Encoder bounce | Already filtered in code - check hardware |
| Pages reverse direction | CLK/DT swapped | Swap GPIO 32↔33 in wiring |
| Slow response | Update interval too long | `SN_LCD_SetUpdateInterval(50)` |

---

## Test 3: Status Page Data Display

### Page Layout
```
Row 0: === ROVER XR-4 ===
Row 1: State: [STATE]
Row 2: [E-STOP] [ARM]
Row 3: Link:[STATUS] OBC:[RSSI]dB  [PAGE]
```

### Test 3A: ARM Switch
1. Navigate to STATUS page (page 1)
2. Toggle ARM switch ON
3. Observe row 2 changes from "DISARM" to "ARMED"
4. Toggle ARM switch OFF
5. Observe row 2 changes back to "DISARM"

### Test 3B: E-STOP Switch
1. Trigger E-STOP (open switch)
2. Observe row 2 shows "E-STOP!"
3. Release E-STOP (close switch)
4. Observe row 2 shows "NORMAL"

### Test 3C: ESP-NOW Connection Indicator
1. Observe "Link:" field on row 3
2. Should show "OK" (blinking every 500ms)
3. OBC RSSI value should update

### ✅ Pass Criteria
- State field updates within 100ms
- ARM status toggles correctly
- E-STOP indicator appears immediately
- Link indicator blinks visibly
- RSSI shows reasonable value (-30 to -80 dBm typical)

---

## Test 4: Telemetry Page Real-Time Updates

### Page Layout
```
Row 0:     TELEMETRY
Row 1: Volt: [XX.XX]V
Row 2: Curr: [X.XX]A
Row 3: Temp:[XX.X]°C   [XX.X]W  [PAGE]
```

### Procedure
1. Navigate to TELEMETRY page (page 2)
2. Observe voltage value updates
3. Observe current value updates
4. Verify power calculation (V × I)
5. Observe temperature updates

### ✅ Pass Criteria
- Voltage updates every 100ms
- Current updates every 100ms
- Power = Voltage × Current (verified manually)
- Temperature updates smoothly
- Degree symbol (°) displays correctly
- Values don't flicker or jitter

### Expected Values (Idle Rover)
- Voltage: 11.0 - 12.6V (typical LiPo battery)
- Current: 0.1 - 1.0A (idle consumption)
- Temperature: 20 - 30°C (room temp)
- Power: 1 - 10W

---

## Test 5: GPS Page

### Page Layout
```
Row 0: GPS: [FIX/NO FIX]
Row 1: Lat: [XX.XXXXXX]
Row 2: Lon: [-XXX.XXXXXX]
Row 3: Time: [XXXXXX.XX]  [PAGE]
```

### Test 5A: No GPS Fix
1. Navigate to GPS page (page 3)
2. If GPS has no fix yet:
   - Row 0: "GPS: NO FIX"
   - Row 1: "Lat: ---.------"
   - Row 2: "Lon: ---.------"
   - Row 3: "Time: --:--:--"

### Test 5B: GPS Fix Acquired
1. Wait for GPS fix (may take 30-60 seconds outdoors)
2. Observe:
   - Row 0: "GPS: FIX"
   - Lat/Lon show real coordinates
   - Time updates continuously

### ✅ Pass Criteria
- Fix status toggles correctly
- Coordinates show 6 decimal places
- Coordinates update when moving
- Time value updates
- No garbled characters

---

## Test 6: Sensors Page

### Page Layout
```
Row 0:      SENSORS
Row 1: G:[X.X] [Y.Y] [Z.Z]
Row 2: A:[X.X] [Y.Y] [Z.Z]
Row 3: M:[XXX] [YYY] [ZZZ]  [PAGE]
```

### Procedure
1. Navigate to SENSORS page (page 4)
2. Hold rover still, observe baseline values
3. Tilt rover left/right, observe gyro/accel change
4. Rotate rover, observe magnetometer change
5. Return to level, verify values return to baseline

### Expected Values (Stationary)
- Gyro: ~0.0, 0.0, 0.0 (deg/s)
- Accel: ~0.0, 0.0, 9.8 (m/s², Z should be ~gravity)
- Mag: Varies by location (Earth's magnetic field)

### ✅ Pass Criteria
- Values update in real-time (100ms)
- Gyro responds to rotation
- Accelerometer responds to tilt
- Magnetometer responds to orientation
- Values return to baseline when stationary

---

## Test 7: Control Page

### Page Layout
```
Row 0:  CONTROL INPUTS
Row 1: Joy X:[XXXX]  Y:[XXXX]
Row 2: Encoder: [XXX]
Row 3: H:[ON/OFF] A:[Y/N] E:[Y/N]  [PAGE]
```

### Test 7A: Joystick
1. Navigate to CONTROL page (page 5)
2. Move joystick left/right → X value changes
3. Move joystick up/down → Y value changes
4. Center joystick → X≈2048, Y≈2048

### Test 7B: Encoder Position
1. On CONTROL page
2. Rotate encoder CW → position increases
3. Rotate encoder CCW → position decreases
4. **Note**: Position changes while navigating other pages too

### Test 7C: Switch Indicators
1. Toggle HEADLIGHTS → H:ON / H:OFF
2. Toggle ARM → A:Y / A:N
3. Trigger E-STOP → E:Y, Release → E:N

### ✅ Pass Criteria
- Joystick values: 0-4095 range
- Center position: ~2000-2100
- Encoder tracks every click
- Switch indicators update instantly

---

## Test 8: Non-Blocking Performance

### Procedure
1. Keep Serial Monitor open
2. Navigate through all pages rapidly
3. Move joystick continuously
4. Monitor for any warnings/errors
5. Verify ESP-NOW link stays "OK"

### Monitor These Metrics
```
Loop Frequency: Should stay > 10,000 Hz
E-STOP Response: Should stay < 100 µs
ESP-NOW Link: Should not disconnect
Serial Output: Should not show timing warnings
```

### Stress Test Procedure
1. Rapidly rotate encoder back and forth
2. Move joystick in circles continuously
3. Toggle ARM and HEADLIGHTS switches
4. Check Serial Monitor for errors
5. Verify all pages still update smoothly

### ✅ Pass Criteria
- No "link timeout" errors
- No "buffer overflow" warnings
- Loop frequency stays > 10kHz
- LCD updates stay smooth
- Encoder navigation stays responsive
- E-STOP response stays < 1ms

---

## Test 9: Update Interval Configuration

### Procedure
1. In `main.cpp` or handler, add after `SN_LCD_Init()`:
   ```cpp
   SN_LCD_SetUpdateInterval(50);  // Test 50ms updates
   ```
2. Upload firmware
3. Observe faster LCD updates (20Hz vs 10Hz)
4. Check Serial Monitor for interval change log
5. Try different intervals: 200ms (slower), 25ms (very fast)

### Expected Serial Output
```
[SN_LCD] Update interval set to 50 ms
```

### ✅ Pass Criteria
- Faster intervals = smoother updates
- Slower intervals = less CPU usage
- No errors at any tested interval
- ESP-NOW still works at all intervals

---

## Test 10: Graceful Degradation (LCD Disconnected)

### Procedure
1. Power off rover
2. Disconnect LCD I2C wires
3. Power on rover
4. Check Serial Monitor

### Expected Serial Output
```
[SN_LCD_Init] Scanning for LCD at address 0x27
[SN_LCD_Init] LCD not found at address 0x27, Error: 2
```

### Expected Behavior
- Rover still boots normally
- ESP-NOW still works
- Encoder navigation still works
- No crashes or hangs
- Serial output confirms LCD missing

### ✅ Pass Criteria
- `SN_LCD_IsReady()` returns `false`
- Rover fully functional without LCD
- No infinite loops or crashes
- Clear error message in Serial

---

## Performance Benchmarks

### Target Performance Metrics
| Metric | Target | Typical | Max Acceptable |
|--------|--------|---------|----------------|
| LCD Update Time | < 5ms | 2-3ms | 10ms |
| Page Change Time | < 100ms | 50ms | 200ms |
| Encoder Response | < 50ms | 20ms | 100ms |
| CPU Usage | < 2% | 0.5-1% | 5% |
| Main Loop Freq | > 10kHz | 40-50kHz | 5kHz |
| ESP-NOW Link | 100% | 100% | 99% |

### How to Measure
```cpp
// Add to SN_CTU_MainHandler() temporarily
static unsigned long perfTestStart = 0;
static unsigned long loopCount = 0;

loopCount++;
if (millis() - perfTestStart >= 1000) {
    Serial.printf("Loop Freq: %lu Hz\n", loopCount);
    loopCount = 0;
    perfTestStart = millis();
}
```

---

## Common Issues & Solutions

### Issue: Pages change too fast when rotating encoder
**Solution**: Encoder may not have detents. Increase debounce in `SN_Switches.h`:
```cpp
#define ENCODER_DEBOUNCE_MS 5  // Increase from 2ms
```

### Issue: LCD flickers on page change
**Solution**: Normal - full clear/redraw. Can reduce flicker by:
1. Using partial updates (advanced)
2. Reducing update interval to 50ms

### Issue: ESP-NOW disconnects during heavy LCD usage
**Solution**: This should NOT happen. If it does:
1. Check `SN_LCD_Update()` is non-blocking
2. Verify no `delay()` calls in LCD code
3. Increase ESP-NOW priority if using FreeRTOS tasks

### Issue: Text doesn't fit on screen
**Solution**: LCD is 20 characters wide. Check render functions:
- STATUS page: Longest line is "=== ROVER XR-4 ===" (18 chars) ✅
- Strings may need trimming if values very large

---

## Final Validation Checklist

- [ ] All 5 pages display correctly
- [ ] Encoder navigation works both directions
- [ ] Page wrapping works (5→1 and 1→5)
- [ ] ARM switch updates STATUS page
- [ ] E-STOP updates STATUS page immediately
- [ ] Voltage/current update on TELEMETRY page
- [ ] GPS coordinates update (if fix available)
- [ ] Sensor values respond to movement
- [ ] Joystick values track stick position
- [ ] Encoder position tracks rotation
- [ ] ESP-NOW stays connected during navigation
- [ ] Loop frequency > 10kHz
- [ ] E-STOP response < 1ms
- [ ] Rover works if LCD disconnected

---

## Success Criteria

### ✅ PASS: LCD Integration Complete
All tests pass with no critical failures. Minor cosmetic issues acceptable.

### ⚠️ PARTIAL: Needs Adjustment
Most tests pass, but some tweaking needed (update intervals, formatting, etc.)

### ❌ FAIL: Major Issues
Critical failures: ESP-NOW disrupted, crashes, non-responsive navigation

---

## Next Steps After Testing

1. **Tune update interval** based on observed performance
2. **Add custom features** using rotary button press
3. **Implement menu system** for settings (advanced)
4. **Add data logging** display page
5. **Create diagnostic page** with error codes

---

## Support

If issues persist after following this guide:
1. Check wiring against pinout diagram
2. Verify all libraries are up to date
3. Test LCD with I2C scanner sketch
4. Check Serial logs for detailed error messages
5. Verify encoder working with standalone test sketch

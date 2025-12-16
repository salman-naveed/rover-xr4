# LCD Page Layout Reference

Visual reference for all 5 LCD pages showing exact character positions and content.

---

## LCD Specifications

- **Size**: 20 columns × 4 rows
- **Character Set**: Standard ASCII + custom characters
- **Update Rate**: 100ms default (configurable)
- **Navigation**: Encoder rotation (CW/CCW)

---

## Page 1: STATUS

```
╔════════════════════╗
║=== ROVER XR-4 ===  ║  Row 0: Title (centered)
║State: ARMED        ║  Row 1: System state (8 chars max)
║NORMAL  ARMED       ║  Row 2: E-STOP (7) + ARM (6)
║Link:OK OBC:-45dB1/5║  Row 3: ESP-NOW status + page
╚════════════════════╝
     20 characters
```

### Character Map (Row by Row)
```
Col: 01234567890123456789
R0:  === ROVER XR-4 ===  
R1:  State: ARMED        
R2:  NORMAL  ARMED       
R3:  Link:OK OBC:-45dB1/5
```

### Dynamic Fields
- **Row 1, Col 7-14**: State name (POWERUP, INIT, COMMS, WAITING, ARMED, ERROR, E-STOP, REBOOT)
- **Row 2, Col 0-6**: E-STOP status (NORMAL or E-STOP!)
- **Row 2, Col 8-13**: ARM status (DISARM or ARMED)
- **Row 3, Col 5-6**: Link blink (OK/ok)
- **Row 3, Col 12-15**: RSSI value (-30 to -90)
- **Row 3, Col 18-20**: Page indicator (1/5)

---

## Page 2: TELEMETRY

```
╔════════════════════╗
║    TELEMETRY       ║  Row 0: Title (centered)
║Volt: 12.34V        ║  Row 1: Battery voltage
║Curr: 2.56A         ║  Row 2: Battery current
║Temp:25.3°C   32.1W ║  Row 3: Temp + Power + page
╚════════════════════╝
```

### Character Map
```
Col: 01234567890123456789
R0:      TELEMETRY       
R1:  Volt: 12.34V        
R2:  Curr: 2.56A         
R3:  Temp:25.3°C   32.1W 
```

### Dynamic Fields
- **Row 1, Col 6-11**: Voltage (XX.XXV format)
- **Row 2, Col 6-10**: Current (X.XXA format)
- **Row 3, Col 5-9**: Temperature (XX.X format)
- **Row 3, Col 10**: Degree symbol (custom char 3)
- **Row 3, Col 14-18**: Power (XX.XW format)
- **Row 3, Col 18-20**: Page indicator (2/5)

### Format Details
- Voltage: 2 decimal places (12.34V)
- Current: 2 decimal places (2.56A)
- Temperature: 1 decimal place (25.3°C)
- Power: 1 decimal place (32.1W)

---

## Page 3: GPS

```
╔════════════════════╗
║GPS: FIX            ║  Row 0: GPS status
║Lat: 37.774929      ║  Row 1: Latitude
║Lon:-122.419418     ║  Row 2: Longitude
║Time: 123456.78   3/5║  Row 3: GPS time + page
╚════════════════════╝
```

### Character Map (With Fix)
```
Col: 01234567890123456789
R0:  GPS: FIX            
R1:  Lat: 37.774929      
R2:  Lon:-122.419418     
R3:  Time: 123456.78   3/5
```

### Character Map (No Fix)
```
Col: 01234567890123456789
R0:  GPS: NO FIX         
R1:  Lat: ---.------     
R2:  Lon: ---.------     
R3:  Time: --:--:--    3/5
```

### Dynamic Fields
- **Row 0, Col 5-11**: Fix status (FIX or NO FIX)
- **Row 1, Col 5-14**: Latitude (6 decimals or dashes)
- **Row 2, Col 5-16**: Longitude (6 decimals or dashes)
- **Row 3, Col 6-15**: GPS time or dashes
- **Row 3, Col 18-20**: Page indicator (3/5)

### Coordinate Format
- Latitude: -90.000000 to 90.000000 (10 chars max)
- Longitude: -180.000000 to 180.000000 (12 chars max)
- Shows `---.------` when no fix

---

## Page 4: SENSORS

```
╔════════════════════╗
║     SENSORS        ║  Row 0: Title (centered)
║G:0.5 -0.2 0.1      ║  Row 1: Gyroscope (X Y Z)
║A:0.0 0.0 9.8       ║  Row 2: Accelerometer (X Y Z)
║M:123 -45 67      4/5║  Row 3: Magnetometer (X Y Z) + page
╚════════════════════╝
```

### Character Map
```
Col: 01234567890123456789
R0:       SENSORS        
R1:  G:0.5 -0.2 0.1      
R2:  A:0.0 0.0 9.8       
R3:  M:123 -45 67      4/5
```

### Dynamic Fields
- **Row 1, Col 2-16**: Gyro X Y Z (1 decimal each)
- **Row 2, Col 2-16**: Accel X Y Z (1 decimal each)
- **Row 3, Col 2-14**: Mag X Y Z (0 decimals, integers)
- **Row 3, Col 18-20**: Page indicator (4/5)

### Value Formats
- **Gyroscope**: ±XXX.X (deg/s, 1 decimal)
- **Accelerometer**: ±XX.X (m/s², 1 decimal)
- **Magnetometer**: ±XXXX (µT, integer)

### Sensor Prefixes
- **G**: Gyroscope (rotation rate)
- **A**: Accelerometer (linear acceleration)
- **M**: Magnetometer (magnetic field)

---

## Page 5: CONTROL

```
╔════════════════════╗
║ CONTROL INPUTS     ║  Row 0: Title (centered)
║Joy X:2048  Y:1987  ║  Row 1: Joystick X & Y
║Encoder: 42         ║  Row 2: Encoder position
║H:ON  A:Y E:N     5/5║  Row 3: Switches + page
╚════════════════════╝
```

### Character Map
```
Col: 01234567890123456789
R0:   CONTROL INPUTS     
R1:  Joy X:2048  Y:1987  
R2:  Encoder: 42         
R3:  H:ON  A:Y E:N     5/5
```

### Dynamic Fields
- **Row 1, Col 6-9**: Joystick X value (0-4095)
- **Row 1, Col 14-17**: Joystick Y value (0-4095)
- **Row 2, Col 9-14**: Encoder position (signed int)
- **Row 3, Col 2-4**: Headlights (ON or OFF)
- **Row 3, Col 8**: ARM status (Y or N)
- **Row 3, Col 12**: E-STOP status (Y or N)
- **Row 3, Col 18-20**: Page indicator (5/5)

### Switch Indicators
- **H**: Headlights (ON/OFF)
- **A**: Armed (Y/N)
- **E**: E-STOP (Y/N)

---

## Custom Characters

### Character Codes
```cpp
lcd.createChar(1, lcd_startup_char);  // Diamond icon
lcd.createChar(2, lcd_arrow_up);      // Up arrow
lcd.createChar(3, lcd_degree);        // Degree symbol (°)
```

### Usage in Code
```cpp
lcd.write(1);  // Print startup diamond
lcd.write(2);  // Print up arrow
lcd.write(3);  // Print degree symbol (°C)
```

### Visual Representation
```
Char 1 (Diamond):     Char 2 (Arrow):      Char 3 (Degree):
  ▄▄                    █                    ██  
 ████                  ███                  █  █ 
 ████                   █                   █  █ 
  ▀▀                    █                    ██  
```

---

## Page Indicator (All Pages)

### Position
- **Column**: 18-20 (last 3 characters)
- **Row**: 3 (bottom row)

### Format
```
1/5  ← Page 1 of 5
2/5  ← Page 2 of 5
3/5  ← Page 3 of 5
4/5  ← Page 4 of 5
5/5  ← Page 5 of 5
```

### Alignment
- Right-aligned in last 3 columns
- Always visible
- Updates on page change

---

## Blink Indicators

### ESP-NOW Link (Page 1)
- **Location**: Row 3, Col 5-6
- **Rate**: 500ms (2Hz)
- **States**: `OK` (bright) ↔ `ok` (dim)
- **Purpose**: Visual confirmation of active connection

### Implementation
```cpp
if (lcd_state.blink_state) {
    lcd.print("OK ");  // Uppercase when blink_state = true
} else {
    lcd.print("ok ");  // Lowercase when blink_state = false
}
```

---

## Data Update Rates

| Page | Field | Update Rate | Source |
|------|-------|-------------|--------|
| STATUS | State | 100ms | System context |
| STATUS | ARM | Event-driven | Switch ISR |
| STATUS | E-STOP | Event-driven | Switch ISR |
| STATUS | Link | 500ms blink | Timer |
| TELEMETRY | Voltage | 100ms | OBC telemetry |
| TELEMETRY | Current | 100ms | OBC telemetry |
| TELEMETRY | Temp | 100ms | OBC telemetry |
| GPS | Fix | 100ms | GPS module |
| GPS | Coords | 1Hz typical | GPS module |
| SENSORS | Gyro | 100ms | IMU sensor |
| SENSORS | Accel | 100ms | IMU sensor |
| SENSORS | Mag | 100ms | Magnetometer |
| CONTROL | Joystick | 100ms | ADC read |
| CONTROL | Encoder | Event-driven | Encoder ISR |
| CONTROL | Switches | Event-driven | Switch ISR |

---

## Character Budget Analysis

### Tightest Fit Pages
1. **GPS Page Row 2**: `Lon:-122.419418` = 17 chars (3 chars margin)
2. **Control Page Row 1**: `Joy X:2048  Y:1987` = 18 chars (2 chars margin)
3. **Sensors Page Row 3**: `M:123 -45 67` = 12 chars (good margin)

### Most Margin
1. **Status Page Row 1**: `State: ARMED` = 12 chars (8 chars margin)
2. **Telemetry Row 2**: `Curr: 2.56A` = 11 chars (9 chars margin)

All pages fit comfortably within 20-character width.

---

## Alignment Patterns

### Left-Aligned
- Most data fields
- Labels and prefixes
- Default for simplicity

### Centered
- Page titles (Row 0)
- Aesthetic for headers

### Right-Aligned
- Page indicators (last 3 chars)
- Could be used for units

---

## Screen Refresh Strategy

### Full Redraw Events
- Page change
- Forced redraw (`SN_LCD_ForceRedraw()`)
- Timer interval elapsed (100ms default)
- Blink state change

### Partial Update (Future)
Could optimize by tracking changed fields:
- Only update voltage if changed > 0.1V
- Only update joystick if delta > 50
- Only update encoder on change

**Current Implementation**: Full redraw every 100ms (simpler, reliable)

---

## Visual Design Principles

### Hierarchy
1. **Title** (Row 0): Identifies page purpose
2. **Primary Data** (Rows 1-2): Main information
3. **Secondary Data** (Row 3): Supporting info + navigation

### Readability
- Clear labels (Volt:, Lat:, etc.)
- Consistent spacing
- Units always shown (V, A, °C, etc.)
- No abbreviations that aren't obvious

### Responsiveness
- Critical data on top
- Navigation hint at bottom
- Blink for active indicators
- Immediate updates on events

---

## Error Handling Display

### GPS No Fix
```
GPS: NO FIX         
Lat: ---.------     
Lon: ---.------     
Time: --:--:--    3/5
```

### No Telemetry Data
Values show last received or 0.00:
```
    TELEMETRY
Volt: 0.00V
Curr: 0.00A
Temp:0.0°C   0.0W  2/5
```

### ESP-NOW Disconnected
```
=== ROVER XR-4 ===
State: ERROR  
E-STOP! DISARM  
Link:-- OBC:---    1/5
```

---

## Future Enhancements

### Additional Pages (Beyond 5)
- **Diagnostics**: Error codes, uptime, reboot count
- **Network**: WiFi status, IP address
- **Storage**: SD card status, log file size
- **Motor**: PWM values, motor current
- **Battery**: Remaining %, estimated runtime

### Interactive Features
- **Button Press Menu**: Use rotary button for sub-menus
- **Value Selection**: Highlight and edit settings
- **Graphing**: Simple bar graphs for trending
- **Alerts**: Flash screen on critical warnings

### Visual Improvements
- More custom characters (battery icon, signal bars, etc.)
- Progress bars using custom chars
- Animated indicators (spinner during GPS fix)
- Contrast adjustment via encoder

---

## Summary

✅ **5 Pages**: Comprehensive coverage of all rover data  
✅ **Clear Layout**: Every character position planned  
✅ **Consistent**: Same structure across all pages  
✅ **Readable**: Labels, units, proper spacing  
✅ **Responsive**: Updates every 100ms  
✅ **Robust**: Handles missing data gracefully  

**Ready for deployment!** 🚀

# LCD Quick Reference

## 5 Pages Overview

| Page | Name | Shows |
|------|------|-------|
| 1 | STATUS | System state, ARM, E-STOP, ESP-NOW link |
| 2 | TELEMETRY | Voltage, Current, Temperature, Power |
| 3 | GPS | Fix status, Latitude, Longitude, Time |
| 4 | SENSORS | Gyro, Accelerometer, Magnetometer |
| 5 | CONTROL | Joystick, Encoder, Switches |

## Navigation
- **Rotate CW**: Next page →
- **Rotate CCW**: Previous page ←
- **Press Button**: Reserved (future use)

## Key Functions

```cpp
// Required in main loop (SN_Handler.cpp)
SN_LCD_Update(&xr4_system_context);  // Call every loop - non-blocking

// Navigation (automatic via encoder)
SN_LCD_NextPage();  // Triggered by encoder CW
SN_LCD_PrevPage();  // Triggered by encoder CCW

// Configuration
SN_LCD_SetUpdateInterval(100);  // milliseconds (default: 100ms)

// Utilities
SN_LCD_ForceRedraw();          // Force refresh
SN_LCD_IsReady();              // Check if LCD available
SN_LCD_GetCurrentPage();       // Get current page (0-4)
```

## Timing Configuration

```cpp
// Fast updates (more CPU usage)
SN_LCD_SetUpdateInterval(50);   // 20Hz refresh

// Default (balanced)
SN_LCD_SetUpdateInterval(100);  // 10Hz refresh ← Recommended

// Slow updates (save CPU)
SN_LCD_SetUpdateInterval(200);  // 5Hz refresh
```

## Integration Checklist

✅ **main.cpp**: Already has `SN_LCD_Init()`  
✅ **SN_Handler.cpp**: Added `SN_LCD_Update()` in main loop  
✅ **SN_Handler.cpp**: Added encoder navigation logic  
✅ **Header included**: `#include <SN_LCD.h>` in handler  

## Performance
- Update time: 2-5ms (when rendering)
- CPU usage: < 1% (at 100ms interval)
- ESP-NOW impact: **ZERO** (non-blocking)

## Troubleshooting

| Problem | Solution |
|---------|----------|
| No display | Check I2C address (0x27), power, wiring |
| Pages not changing | Verify encoder wiring (GPIO 32/33) |
| Slow updates | Reduce interval to 50ms |
| Garbled text | Check 5V power, I2C pull-ups |

## Display Format Examples

### STATUS Page
```
=== ROVER XR-4 ===
State: ARMED  
NORMAL  ARMED  
Link:OK OBC:-45dB
```

### TELEMETRY Page
```
    TELEMETRY
Volt: 12.34V
Curr: 2.56A
Temp:25.3°C   32.1W
```

### GPS Page
```
GPS: FIX  
Lat: 37.774929
Lon:-122.419418
Time: 123456.78
```

## Custom Characters
- `\x01` - Startup icon (diamond)
- `\x02` - Up arrow
- `\x03` - Degree symbol (°)

## Page Number
All pages show page indicator in bottom-right:  
`1/5`, `2/5`, `3/5`, `4/5`, `5/5`

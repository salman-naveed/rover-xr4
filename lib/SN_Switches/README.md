# SN_Switches Library - CTU Switch Handler

## Overview

The `SN_Switches` library provides robust, debounced switch input handling for the XR4 Rover Control & Telemetry Unit (CTU). It implements:

- **Hardware interrupts** for safety-critical inputs (E-STOP)
- **Hardware interrupts** for responsive encoder tracking
- **Software debouncing** for all switches
- **Edge detection** for state changes
- **Thread-safe** interrupt handlers

## Hardware Configuration

### GPIO Pin Assignments

| Component | Pin | Type | Configuration |
|-----------|-----|------|---------------|
| ARM Switch | GPIO 4 | Digital In | External 10kΩ pullup to 3.3V |
| HEADLIGHTS Switch | GPIO 19 | Digital In | External 10kΩ pullup to 3.3V |
| E-STOP Switch | GPIO 14 | Digital In | NC switch, external 10kΩ pullup |
| Rotary CLK | GPIO 32 | Digital In | Internal pullup (INPUT_PULLUP) |
| Rotary DT | GPIO 33 | Digital In | Internal pullup (INPUT_PULLUP) |
| Rotary SW | GPIO 23 | Digital In | Internal pullup (INPUT_PULLUP) |

### E-STOP Wiring Details

**CRITICAL**: E-STOP is wired as **Normally Closed (NC)**

- **Normal Operation**: Switch closed → GPIO pulled to GND (LOW)
- **E-STOP Triggered**: Switch open → GPIO pulled HIGH by 10kΩ resistor
- **Broken Wire**: Also triggers E-STOP (fail-safe design)

### Switch Wiring

All switches (ARM, HEADLIGHTS):
- Switch connects GPIO to GND when pressed/closed
- 10kΩ external pullup to 3.3V
- GPIO reads HIGH when open, LOW when closed

### Rotary Encoder

Standard quadrature encoder with push button:
- CLK and DT pins for rotation detection
- SW pin for button press
- Internal pullups enabled (INPUT_PULLUP)
- Active LOW button (pressed = LOW)

## Features

### 1. **E-STOP Safety Features**

- ✅ **Hardware Interrupt**: Responds within microseconds
- ✅ **Fail-Safe Design**: NC switch - broken wire triggers E-STOP
- ✅ **20ms Debounce**: Fast enough for safety, eliminates false triggers
- ✅ **Direct System Context Update**: Bypasses main loop for immediate response
- ✅ **Both Edge Detection**: Detects activation AND release

### 2. **ARM Switch**

- ✅ **30ms Debounce**: Eliminates contact bounce
- ✅ **Edge Detection**: Detects state changes
- ✅ **System Integration**: Updates `xr4_system_context.Armed`
- ✅ **State Machine Integration**: Triggers ARM/DISARM transitions

### 3. **HEADLIGHTS Switch**

- ✅ **30ms Debounce**: Smooth operation
- ✅ **Edge Detection**: Detects ON/OFF changes
- ✅ **Responsive**: No perceived lag
- ✅ **System Integration**: Updates `xr4_system_context.Headlights_On`

### 4. **Rotary Encoder**

- ✅ **Hardware Interrupt**: Immediate rotation tracking
- ✅ **Quadrature Decoding**: Accurate direction detection
- ✅ **5ms Debounce**: Minimal delay, no missed steps
- ✅ **Position Tracking**: Absolute and delta (change) values
- ✅ **Button Press Detection**: With 30ms debounce
- ✅ **Overflow Safe**: 16-bit signed position (-32768 to +32767)

## API Reference

### Initialization

```cpp
void SN_Switches_Init();
```

Initialize all switches, configure GPIO pins, and attach interrupts.  
**Call once in `setup()`**.

### Update Function

```cpp
void SN_Switches_Update();
```

Update switch states with debouncing (for non-interrupt switches).  
**Call frequently in `loop()`** (recommended: every loop iteration).

Note: E-STOP and encoder are handled by interrupts and don't require `Update()` to function, but calling it ensures consistent state and logging.

### Get All States

```cpp
SwitchStates_t SN_Switches_GetStates();
```

Returns a structure containing all switch states and flags:

```cpp
typedef struct {
    // Current switch states (debounced)
    bool arm_switch;
    bool headlights_switch;
    bool estop_switch;
    bool rotary_button;
    
    // Rotary encoder
    int16_t encoder_position;  // Absolute position
    int8_t encoder_delta;      // Change since last read
    
    // Event flags (one-shot, cleared after reading)
    bool estop_triggered;      // E-STOP was activated
    bool arm_changed;          // ARM switch changed
    bool headlights_changed;   // HEADLIGHTS changed
    bool rotary_pressed;       // Rotary button pressed
} SwitchStates_t;
```

**Important**: Event flags are cleared after reading with `GetStates()`.

### Individual Getters

```cpp
bool SN_Switches_IsEStopActive();      // Current E-STOP state
bool SN_Switches_IsArmed();            // Current ARM state
bool SN_Switches_AreHeadlightsOn();    // Current HEADLIGHTS state
int16_t SN_Switches_GetEncoderPosition(); // Absolute encoder position
int8_t SN_Switches_GetEncoderDelta();  // Encoder change (cleared after read)
```

### Encoder Control

```cpp
void SN_Switches_ResetEncoder();
```

Reset encoder position to 0.

## Usage Examples

### Basic Setup

```cpp
#include <SN_Switches.h>

void setup() {
    SN_Switches_Init();
}

void loop() {
    SN_Switches_Update();
    
    // Your code here
}
```

### E-STOP Check (Safety Critical)

```cpp
void loop() {
    SN_Switches_Update();
    
    // ALWAYS check E-STOP first
    if (SN_Switches_IsEStopActive()) {
        // STOP EVERYTHING
        SN_Motors_Stop();
        return; // Exit immediately
    }
    
    // Safe to proceed
    // ... normal operation ...
}
```

### ARM/DISARM Detection

```cpp
void loop() {
    SN_Switches_Update();
    
    SwitchStates_t states = SN_Switches_GetStates();
    
    if (states.arm_changed) {
        if (states.arm_switch) {
            Serial.println("System ARMED");
            xr4_system_context.system_state = XR4_STATE_ARMED;
        } else {
            Serial.println("System DISARMED");
            xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
        }
    }
}
```

### Headlights Control

```cpp
void loop() {
    SN_Switches_Update();
    
    SwitchStates_t states = SN_Switches_GetStates();
    
    if (states.headlights_changed) {
        if (states.headlights_switch) {
            // Send command to turn on headlights
            xr4_system_context.Headlights_On = true;
        } else {
            xr4_system_context.Headlights_On = false;
        }
    }
}
```

### Rotary Encoder - Menu Navigation

```cpp
void loop() {
    SN_Switches_Update();
    
    static int menuItem = 0;
    SwitchStates_t states = SN_Switches_GetStates();
    
    // Scroll menu
    if (states.encoder_delta != 0) {
        menuItem += states.encoder_delta;
        if (menuItem < 0) menuItem = 0;
        if (menuItem > 9) menuItem = 9;
        Serial.printf("Menu Item: %d\n", menuItem);
    }
    
    // Select menu item
    if (states.rotary_pressed) {
        Serial.printf("Selected: %d\n", menuItem);
    }
}
```

### Rotary Encoder - Value Adjustment

```cpp
void loop() {
    SN_Switches_Update();
    
    static float brightness = 50.0; // 0-100%
    
    int8_t delta = SN_Switches_GetEncoderDelta();
    if (delta != 0) {
        brightness += (delta * 5.0); // 5% per click
        brightness = constrain(brightness, 0.0, 100.0);
        Serial.printf("Brightness: %.1f%%\n", brightness);
    }
}
```

## System Integration

The library automatically updates `xr4_system_context_t`:

```cpp
xr4_system_context.Emergency_Stop  // E-STOP state
xr4_system_context.Armed           // ARM state
xr4_system_context.Headlights_On   // HEADLIGHTS state
xr4_system_context.Encoder_Pos     // Encoder position (uint16_t)
```

These values are transmitted to the OBC via ESP-NOW.

## Timing Characteristics

| Feature | Response Time | Notes |
|---------|---------------|-------|
| E-STOP Interrupt | < 1 ms | Hardware interrupt + 20ms debounce |
| Encoder Interrupt | < 1 ms | Hardware interrupt + 5ms debounce |
| ARM Switch | ~30 ms | Software debounce in main loop |
| HEADLIGHTS Switch | ~30 ms | Software debounce in main loop |
| Main Loop Update | Variable | Depends on loop() execution time |

## Best Practices

### 1. **Always Check E-STOP First**
```cpp
if (SN_Switches_IsEStopActive()) {
    // STOP EVERYTHING
    return;
}
```

### 2. **Call Update() Every Loop**
```cpp
void loop() {
    SN_Switches_Update(); // First thing in loop
    // ... rest of code ...
}
```

### 3. **Use Event Flags for State Changes**
```cpp
SwitchStates_t states = SN_Switches_GetStates();
if (states.arm_changed) {
    // React to change
}
```

### 4. **Handle Encoder Delta**
```cpp
int8_t delta = SN_Switches_GetEncoderDelta();
// Delta is cleared after reading
```

### 5. **Reset Encoder When Needed**
```cpp
// When entering a new menu or mode
SN_Switches_ResetEncoder();
```

## Debugging

Enable logging in `SN_Switches.cpp` to see switch events:

```cpp
// Already enabled in implementation
logMessage(true, "SN_Switches_Update", "ARM switch %s", ...);
logMessage(true, "SN_Switches_Update", "HEADLIGHTS %s", ...);
logMessage(true, "SN_Switches_Update", "*** E-STOP TRIGGERED ***");
```

## Troubleshooting

### E-STOP Always Active
- Check wiring: NC switch should be CLOSED in normal state
- Verify 10kΩ pullup resistor is connected
- Check GPIO 14 reads LOW when switch is closed

### Switch Not Responding
- Verify correct GPIO pin numbers
- Check external pullup resistors (10kΩ to 3.3V)
- Increase debounce time if needed

### Encoder Counts Incorrectly
- Check CLK and DT wiring
- Verify both pins use INPUT_PULLUP
- Ensure good quality encoder (mechanical encoders can be noisy)

### Encoder Misses Steps
- Reduce `ENCODER_DEBOUNCE_MS` (currently 5ms)
- Check for main loop delays that might block interrupts
- Verify interrupt is attached correctly

## Advanced Configuration

### Adjust Debounce Timing

Edit `SN_Switches.h`:

```cpp
#define DEBOUNCE_DELAY_MS     30    // Regular switches
#define ESTOP_DEBOUNCE_MS     20    // E-STOP
#define ENCODER_DEBOUNCE_MS   5     // Encoder
```

### Hardware RC Filter (Recommended for E-STOP)

Add between GPIO and switch:
- 10kΩ resistor in series
- 100nF capacitor to GND
- Creates ~1ms RC time constant

## Safety Considerations

1. **E-STOP is Fail-Safe**: Broken wire = E-STOP triggered
2. **Interrupt Priority**: E-STOP interrupt has highest priority
3. **Direct Context Update**: E-STOP bypasses main loop
4. **Always Check First**: Check E-STOP before ANY motor operation
5. **Test Regularly**: Test E-STOP function before each operation

## Performance

- **RAM Usage**: ~150 bytes
- **Flash Usage**: ~3 KB
- **CPU Overhead**: Minimal (interrupts + debounce checks)
- **Interrupt Latency**: < 50 μs

## Compatibility

- **Board**: ESP32 (CTU configuration)
- **Arduino Framework**: Yes
- **FreeRTOS**: Compatible (uses ISR-safe functions)
- **ESP-IDF**: Compatible

## License

Part of the XR4 Rover project.

## Version History

- **v1.0** (2025-12-15): Initial implementation
  - E-STOP with interrupt and fail-safe design
  - ARM and HEADLIGHTS with debouncing
  - Rotary encoder with quadrature decoding
  - Full system integration

# SN_Switches Quick Reference

## Setup (One Time)

```cpp
#include <SN_Switches.h>

void setup() {
    SN_Switches_Init();  // Initialize switches
}
```

## Main Loop (Every Iteration)

```cpp
void loop() {
    SN_Switches_Update();  // Update debounced states
    
    // Your code here
}
```

## Common Patterns

### 🚨 E-STOP Check (ALWAYS FIRST!)

```cpp
if (SN_Switches_IsEStopActive()) {
    SN_Motors_Stop();
    return;  // Exit immediately
}
```

### 🔒 ARM/DISARM

```cpp
SwitchStates_t states = SN_Switches_GetStates();

if (states.arm_changed) {
    if (states.arm_switch) {
        Serial.println("ARMED");
    } else {
        Serial.println("DISARMED");
    }
}
```

### 💡 Headlights

```cpp
if (states.headlights_changed) {
    xr4_system_context.Headlights_On = states.headlights_switch;
}
```

### 🎚️ Rotary Encoder - Menu

```cpp
static int menu = 0;

if (states.encoder_delta != 0) {
    menu += states.encoder_delta;
    menu = constrain(menu, 0, MAX_MENU);
}

if (states.rotary_pressed) {
    selectMenuItem(menu);
}
```

### 📊 Rotary Encoder - Value

```cpp
static float value = 50.0;

int8_t delta = SN_Switches_GetEncoderDelta();
if (delta != 0) {
    value += (delta * 5.0);  // 5 units per click
    value = constrain(value, 0, 100);
}
```

## API Quick List

| Function | Returns | Purpose |
|----------|---------|---------|
| `SN_Switches_Init()` | void | Initialize (call in setup) |
| `SN_Switches_Update()` | void | Update states (call in loop) |
| `SN_Switches_GetStates()` | struct | Get all states + events |
| `SN_Switches_IsEStopActive()` | bool | E-STOP status |
| `SN_Switches_IsArmed()` | bool | ARM switch status |
| `SN_Switches_AreHeadlightsOn()` | bool | HEADLIGHTS status |
| `SN_Switches_GetEncoderPosition()` | int16_t | Absolute encoder count |
| `SN_Switches_GetEncoderDelta()` | int8_t | Change since last read |
| `SN_Switches_ResetEncoder()` | void | Reset encoder to 0 |

## SwitchStates_t Structure

```cpp
typedef struct {
    // Current states
    bool arm_switch;
    bool headlights_switch;
    bool estop_switch;
    bool rotary_button;
    
    // Encoder
    int16_t encoder_position;  // -32768 to +32767
    int8_t encoder_delta;      // -1, 0, or +1
    
    // Events (one-shot flags)
    bool estop_triggered;      // ⚠️ E-STOP activated
    bool arm_changed;          // ARM state changed
    bool headlights_changed;   // HEADLIGHTS changed
    bool rotary_pressed;       // Button pressed
} SwitchStates_t;
```

## GPIO Pins

| Switch | GPIO | Wiring |
|--------|------|--------|
| ARM | 4 | External 10kΩ pullup |
| HEADLIGHTS | 19 | External 10kΩ pullup |
| E-STOP | 14 | NC switch, ext pullup |
| Encoder CLK | 32 | Internal pullup |
| Encoder DT | 33 | Internal pullup |
| Encoder SW | 23 | Internal pullup |

## Important Notes

⚠️ **E-STOP**: Normally Closed (NC) - Open = TRIGGERED  
⏱️ **Timing**: E-STOP responds in < 1ms (interrupt)  
🔄 **Events**: Cleared after `GetStates()` - read once per event  
🎯 **Delta**: `encoder_delta` cleared after reading  

## Complete Example

```cpp
#include <SN_Switches.h>

void setup() {
    Serial.begin(115200);
    SN_Switches_Init();
}

void loop() {
    SN_Switches_Update();
    
    // SAFETY FIRST
    if (SN_Switches_IsEStopActive()) {
        Serial.println("E-STOP!");
        return;
    }
    
    // Get states
    SwitchStates_t s = SN_Switches_GetStates();
    
    // Handle events
    if (s.arm_changed) {
        Serial.printf("ARM: %s\n", s.arm_switch ? "ON" : "OFF");
    }
    
    if (s.headlights_changed) {
        Serial.printf("LIGHTS: %s\n", s.headlights_switch ? "ON" : "OFF");
    }
    
    if (s.encoder_delta != 0) {
        Serial.printf("Encoder: %d (Δ=%d)\n", 
                     s.encoder_position, s.encoder_delta);
    }
    
    if (s.rotary_pressed) {
        Serial.println("Button pressed!");
    }
}
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| E-STOP always active | Check NC wiring (should be closed normally) |
| Switch not responding | Verify pullup resistors (10kΩ to 3.3V) |
| Encoder misses steps | Reduce `ENCODER_DEBOUNCE_MS` in header |
| Bouncy switches | Increase `DEBOUNCE_DELAY_MS` in header |

## Remember

1. ✅ Call `Init()` once in `setup()`
2. ✅ Call `Update()` every `loop()`
3. ✅ Check E-STOP before motor operations
4. ✅ Event flags are one-shot (cleared after read)
5. ✅ `encoder_delta` is cleared after reading

# XR-4 CTU Startup Sequence & State Machine

## Overview
This document describes the complete startup sequence and state machine behavior of the XR-4 Control & Telemetry Unit (CTU) firmware.

## Startup Sequence

### Phase 1: Hardware Initialization (setup())

```
Power On
    ↓
Set state = XR4_STATE_JUST_POWERED_ON
    ↓
Initialize Serial Monitor (UART SLIP)
    ↓
Log reset reason
    ↓
Initialize Status Panel (LEDs)
    ↓
Initialize CTU-specific peripherals:
    ├── SN_Switches_Init()      - E-STOP, ARM, Headlights (interrupt-driven)
    ├── SN_Joystick_Init()      - ADC joystick with calibration
    ├── SN_Input_Init()         - Legacy button inputs (GPIO 25, 26, 27)
    └── SN_LCD_Init()           - 20x4 LCD with I2C
        └── Shows splash: "ROVER XR-4 Initializing..." (1.5s)
    ↓
Set state = XR4_STATE_INITIALIZED
    ↓
Exit to loop()
```

**Duration:** ~2-3 seconds

### Phase 2: Communications Setup (loop() - First Iterations)

```
loop() Iteration 1:
    State: XR4_STATE_INITIALIZED
    ↓
    Set LED: Solid Blue (Initializing)
    ↓
    Change state → XR4_STATE_COMMS_CONFIG
    ↓
    Exit (state changed)

loop() Iteration 2:
    State: XR4_STATE_COMMS_CONFIG
    ↓
    Call SN_ESPNOW_Init()
        ├── Configure WiFi as Station
        ├── Initialize ESP-NOW protocol
        ├── Register send callback
        ├── Add OBC as peer
        └── Register receive callback
    ↓
    ┌─────────────────┐
    │  Success?       │
    └────┬───────┬────┘
         │       │
        YES     NO
         │       │
         ↓       ↓
    espnow_     espnow_
    init_       init_
    success     success
    = true      = false
         │       │
    Set state   Set state
    → WAITING   → ERROR
    FOR_ARM
         │       │
    Set LED:    Set LED:
    Moving      Solid
    Back/Forth  Red
         │       │
    Exit loop   Exit loop
```

**Duration:** ~100-500ms

### Phase 3: Normal Operation

```
loop() Iteration 3+:
    State: XR4_STATE_WAITING_FOR_ARM (or ERROR)
    ↓
    Call SN_CTU_MainHandler()
        ├── SN_Switches_Update()           - Debounce switches
        ├── SN_Telemetry_updateContext()   - Process received telemetry
        ├── SN_CTU_ControlInputsHandler()  - Read joystick & switches
        ├── SN_Telecommand_updateStruct()  - Build telecommand packet
        ├── SN_ESPNOW_SendTelecommand()    - Send to OBC
        └── SN_LCD_Update()                - Update display (non-blocking)
    ↓
    Repeat continuously
```

**Loop Frequency:** ~100-200 Hz (depends on handler execution time)

## State Machine Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                      POWER ON / RESET                           │
└──────────────────────────────┬──────────────────────────────────┘
                               ↓
                    ┌──────────────────────┐
                    │ XR4_STATE_JUST_      │
                    │ POWERED_ON           │
                    │ (setup() only)       │
                    └──────────┬───────────┘
                               ↓
                    ┌──────────────────────┐
                    │ XR4_STATE_           │
                    │ INITIALIZED          │
                    │ LED: Solid Blue      │
                    └──────────┬───────────┘
                               ↓
                    ┌──────────────────────┐
                    │ XR4_STATE_           │
                    │ COMMS_CONFIG         │
                    │ (ESP-NOW Init)       │
                    └──────┬───────┬───────┘
                           │       │
                      SUCCESS     FAIL
                           │       │
                           ↓       ↓
              ┌────────────────┐  ┌─────────────────┐
              │ XR4_STATE_     │  │ XR4_STATE_      │
              │ WAITING_FOR_   │  │ ERROR           │
              │ ARM            │  │ LED: Solid Red  │
              │ LED: Moving ↔  │  │ Motors: STOP    │
              └────────┬───────┘  └─────────────────┘
                       │
              ┌────────┴────────┐
              │                 │
         ARM SWITCH        ARM SWITCH
           ACTIVE           INACTIVE
              │                 │
              ↓                 ↓
      ┌───────────────┐   ┌──────────────┐
      │ XR4_STATE_    │   │ Stay in      │
      │ ARMED         │   │ WAITING_FOR_ │
      │ LED: Solid    │◄──┤ ARM state    │
      │ Green         │   │              │
      └───────┬───────┘   └──────────────┘
              │
      ┌───────┴────────┐
      │                │
   E-STOP          E-STOP
   PRESSED        RELEASED
      │                │
      ↓                ↓
┌─────────────────┐   Return to
│ XR4_STATE_      │   appropriate
│ EMERGENCY_STOP  │   state
│ LED: Blink Red  │
│ Motors: STOP    │
└─────────────────┘
```

## State Descriptions

### XR4_STATE_JUST_POWERED_ON (0)
- **When:** Immediately after reset
- **Duration:** Only during `setup()`
- **Actions:** Hardware initialization
- **Next State:** XR4_STATE_INITIALIZED

### XR4_STATE_INITIALIZED (1)
- **When:** After `setup()` completes successfully
- **Duration:** One loop iteration (~10ms)
- **LED:** Solid Blue
- **Actions:** Set LED color
- **Next State:** XR4_STATE_COMMS_CONFIG

### XR4_STATE_COMMS_CONFIG (2)
- **When:** After INITIALIZED state
- **Duration:** One loop iteration (~100-500ms)
- **LED:** Solid Blue
- **Actions:** Initialize ESP-NOW communication
- **Next State:** 
  - Success → XR4_STATE_WAITING_FOR_ARM
  - Failure → XR4_STATE_ERROR

### XR4_STATE_WAITING_FOR_ARM (3)
- **When:** ESP-NOW initialized, waiting for ARM switch
- **Duration:** Until ARM switch activated
- **LED:** Moving Back & Forth (pulsing)
- **Actions:** 
  - Run CTU main handler
  - Send telecommands to OBC
  - Update LCD display
  - Monitor switches
- **Transitions:**
  - ARM switch ON → XR4_STATE_ARMED
  - E-STOP pressed → XR4_STATE_EMERGENCY_STOP
- **Next State:** XR4_STATE_ARMED (when armed)

### XR4_STATE_ARMED (4)
- **When:** ARM switch is active
- **Duration:** While ARM switch remains active
- **LED:** Solid Green
- **Actions:** 
  - Run CTU main handler
  - Send telecommands to OBC (including ARM flag)
  - Update LCD display
  - Monitor switches
- **Transitions:**
  - ARM switch OFF → XR4_STATE_WAITING_FOR_ARM
  - E-STOP pressed → XR4_STATE_EMERGENCY_STOP
- **Next State:** XR4_STATE_WAITING_FOR_ARM (when disarmed)

### XR4_STATE_ERROR (5)
- **When:** Critical error (e.g., ESP-NOW init failure)
- **Duration:** Indefinite (requires reset)
- **LED:** Solid Red
- **Actions:** 
  - Stop all motors (if OBC)
  - Display error on LCD
- **Recovery:** Manual reset required

### XR4_STATE_EMERGENCY_STOP (6)
- **When:** E-STOP switch pressed
- **Duration:** While E-STOP active
- **LED:** Blinking Red
- **Actions:** 
  - Stop all motors immediately (if OBC)
  - Send E-STOP flag to OBC
  - Update LCD showing E-STOP status
- **Transitions:**
  - E-STOP released → Return to previous state
- **Next State:** XR4_STATE_WAITING_FOR_ARM or XR4_STATE_ARMED

### XR4_STATE_REBOOT (8)
- **When:** Commanded by software
- **Duration:** ~100ms
- **LED:** Blinking Yellow
- **Actions:** 
  - Log reboot message
  - Wait 100ms
  - Call ESP.restart()
- **Next State:** XR4_STATE_JUST_POWERED_ON (after reboot)

## Timing Analysis

### Setup Phase Breakdown
```
Component              Duration    Notes
─────────────────────────────────────────────────────────────
UART SLIP Init         ~50ms      Serial @ 115200 baud
Status Panel Init      ~100ms     LED PWM initialization
SN_Switches_Init       ~50ms      GPIO + interrupt setup
SN_Joystick_Init       ~100ms     ADC + calibration load
SN_Input_Init          ~10ms      GPIO configuration
SN_LCD_Init            ~1500ms    I2C + splash screen
─────────────────────────────────────────────────────────────
TOTAL SETUP            ~1.8s      
```

### Loop State Durations
```
State                  Typical Duration    Max Duration
────────────────────────────────────────────────────────
INITIALIZED            10ms                10ms (one pass)
COMMS_CONFIG           150ms               500ms (retry)
WAITING_FOR_ARM        Continuous          -
ARMED                  Continuous          -
ERROR                  Continuous          -
EMERGENCY_STOP         Continuous          -
```

### Handler Execution Time
```
Function                    Execution Time
──────────────────────────────────────────────
SN_Switches_Update()        ~0.5ms
SN_Joystick_ReadRawADC()    ~1.0ms (with averaging)
SN_ESPNOW_SendTelecommand() ~2.0ms
SN_LCD_Update()             ~5.0ms (when updating)
──────────────────────────────────────────────
Total Handler (typical)     ~10ms
Loop Frequency              ~100 Hz
```

## LCD Display During Startup

### Stage 1: Splash Screen (during SN_LCD_Init)
```
┌────────────────────┐
│  === ROVER XR-4 ===│
│   Initializing...  │
│                    │
│                    │
└────────────────────┘
Duration: 1.5 seconds
```

### Stage 2: Status Page (after ESP-NOW init)
```
┌────────────────────┐
│=== ROVER XR-4 ===  │
│State: WAIT  DISARM │
│NORMAL  DISARM      │
│Link:NO CONN        │
└────────────────────┘
If ESP-NOW fails
```

```
┌────────────────────┐
│=== ROVER XR-4 ===  │
│State: WAIT  DISARM │
│NORMAL  DISARM      │
│Link:OK OBC:-45dB   │
└────────────────────┘
If ESP-NOW succeeds
```

## Critical Variables

### Global State Flags
```cpp
// In main.cpp
bool espnow_init_success = false;  // Set true when ESP-NOW initializes

// In xr4_system_context
uint8_t system_state;              // Current state machine state
bool Emergency_Stop;               // E-STOP switch status
bool Armed;                        // ARM switch status
```

### State Transitions
All state changes occur in `main.cpp` loop():
```cpp
switch (xr4_system_context.system_state) {
    case XR4_STATE_INITIALIZED:
        xr4_system_context.system_state = XR4_STATE_COMMS_CONFIG;
        break;
    
    case XR4_STATE_COMMS_CONFIG:
        if (SN_ESPNOW_Init()) {
            espnow_init_success = true;
            xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
        } else {
            espnow_init_success = false;
            xr4_system_context.system_state = XR4_STATE_ERROR;
        }
        break;
    // ...
}
```

## Troubleshooting

### Problem: LCD stuck on "Initializing..."
**Cause:** State machine not advancing
**Solution:** Check that `setup()` sets `system_state = XR4_STATE_INITIALIZED`

### Problem: LCD shows "Link: NO CONN" permanently
**Cause:** ESP-NOW initialization failing
**Solutions:**
1. Check Serial Monitor for ESP-NOW error messages
2. Verify WiFi is configured as Station mode
3. Check OBC MAC address is correct
4. Verify OBC is powered on and running

### Problem: State machine stuck in ERROR
**Cause:** ESP-NOW init returned false
**Solution:** 
1. Check Serial logs for specific failure point
2. Common causes:
   - WiFi already initialized in wrong mode
   - Peer already registered
   - MAC address mismatch
3. Power cycle to reset

### Problem: Rapid state transitions
**Cause:** Switch debouncing not working
**Solution:** Verify `SN_Switches_Update()` is being called regularly

## Serial Monitor Diagnostic Output

Expected output during normal startup:
```
[INFO] Main Logger: setup() - start
[INFO] Main Logger: Last reset: POWERON (1)
[INFO] SN_StatusPanel__Init: Status Panel initialized
[INFO] SN_Switches_Init: Switches initialized with interrupts
[INFO] SN_Joystick_Init: Joystick initialized
[INFO] SN_Input_Init: Joystick and legacy button pins initialized
[INFO] SN_LCD_Init: LCD initialized successfully
[INFO] Main Logger: System initialized - ESP-NOW will initialize in main loop
[INFO] Main Logger: setup() - end
[INFO] SN_ESPNOW_Init: Starting ESP-NOW initialization...
[INFO] SN_WiFi_StartAsWiFiClient: WiFi started as client
[INFO] SN_ESPNOW_Init: ESP-NOW successfully initialized
[INFO] Main Loop: ESP-NOW initialized successfully in COMMS_CONFIG state
```

## Version History

- **v1.0** (Dec 15, 2025): Initial documentation
  - Documented fixed startup sequence
  - Removed broken `esp_init_success` check
  - Simplified LCD initialization

---

**Related Documentation:**
- [LCD Integration Guide](../lib/SN_LCD/LCD_INTEGRATION_GUIDE.md)
- [Switch System Documentation](../lib/SN_Switches/README.md)
- [Joystick Enhancements](../lib/SN_Joystick/JOYSTICK_ENHANCEMENTS_GUIDE.md)

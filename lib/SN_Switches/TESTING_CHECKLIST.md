# SN_Switches Testing Checklist

## Pre-Testing Setup

### Hardware Connections
- [ ] ARM switch connected: GPIO4 to GND with 10kΩ pullup to 3.3V
- [ ] HEADLIGHTS switch connected: GPIO19 to GND with 10kΩ pullup to 3.3V
- [ ] E-STOP switch (NC) connected: GPIO14 with 10kΩ pullup to 3.3V
- [ ] Rotary encoder CLK connected to GPIO32
- [ ] Rotary encoder DT connected to GPIO33
- [ ] Rotary encoder SW connected to GPIO23
- [ ] All GND connections verified
- [ ] All 3.3V pullup connections verified

### Software Setup
- [ ] Code compiled successfully for CTU_ESP32
- [ ] Serial monitor ready at 115200 baud
- [ ] Logging enabled in SN_Logger

## Phase 1: Basic Initialization

### Test 1.1: Power-On Initialization
```
Expected Output:
- "SN_Switches_Init: Initializing CTU switches..."
- "SN_Switches_Init: Switches initialized successfully"
- "Initial states - ARM: X, HEADLIGHTS: X, E-STOP: X"
```

- [ ] Initialization messages appear
- [ ] No error messages
- [ ] Initial states logged

### Test 1.2: Initial State Reading
```cpp
// In setup() after Init()
Serial.printf("E-STOP: %d\n", SN_Switches_IsEStopActive());
Serial.printf("ARM: %d\n", SN_Switches_IsArmed());
Serial.printf("HEADLIGHTS: %d\n", SN_Switches_AreHeadlightsOn());
```

- [ ] E-STOP reads FALSE (switch closed/normal)
- [ ] ARM reads current physical state
- [ ] HEADLIGHTS reads current physical state

## Phase 2: E-STOP Testing (CRITICAL)

### Test 2.1: E-STOP Normal State
- [ ] E-STOP switch closed (normal operation)
- [ ] `SN_Switches_IsEStopActive()` returns FALSE
- [ ] System can operate normally

### Test 2.2: E-STOP Activation
- [ ] Press/open E-STOP switch
- [ ] See log: "*** E-STOP TRIGGERED ***"
- [ ] `SN_Switches_IsEStopActive()` returns TRUE
- [ ] `xr4_system_context.Emergency_Stop` is TRUE
- [ ] Response time < 50ms (observe Serial output timing)

### Test 2.3: E-STOP Release
- [ ] Close E-STOP switch
- [ ] `SN_Switches_IsEStopActive()` returns FALSE
- [ ] `xr4_system_context.Emergency_Stop` is FALSE
- [ ] System recovers to normal state

### Test 2.4: E-STOP Fail-Safe (CRITICAL SAFETY TEST)
- [ ] Disconnect E-STOP wire while monitoring
- [ ] System immediately detects E-STOP (wire open = triggered)
- [ ] Log shows E-STOP triggered
- [ ] **This confirms fail-safe operation**

### Test 2.5: E-STOP Interrupt Performance
```cpp
// Add timing test
unsigned long t1 = micros();
// Trigger E-STOP
// Check when ISR updates flag
unsigned long t2 = micros();
Serial.printf("E-STOP response: %lu us\n", t2 - t1);
```
- [ ] Response time < 1000 microseconds (1ms)

## Phase 3: ARM Switch Testing

### Test 3.1: ARM Switch Basic Operation
- [ ] Flip ARM switch ON
- [ ] See log: "ARM switch ARMED"
- [ ] `SN_Switches_IsArmed()` returns TRUE
- [ ] `xr4_system_context.Armed` is TRUE

- [ ] Flip ARM switch OFF
- [ ] See log: "ARM switch DISARMED"
- [ ] `SN_Switches_IsArmed()` returns FALSE
- [ ] `xr4_system_context.Armed` is FALSE

### Test 3.2: ARM Change Detection
```cpp
SwitchStates_t s = SN_Switches_GetStates();
if (s.arm_changed) {
    Serial.println("ARM CHANGED!");
}
```
- [ ] Flag `arm_changed` is TRUE on state change
- [ ] Flag is FALSE on subsequent reads (one-shot)

### Test 3.3: ARM Debouncing
- [ ] Quickly flip ARM switch multiple times
- [ ] Only see one log message per stable state
- [ ] No bouncing/flickering in state

### Test 3.4: State Machine Integration
- [ ] ARM switch triggers transition to XR4_STATE_ARMED
- [ ] DISARM triggers transition to XR4_STATE_WAITING_FOR_ARM
- [ ] State changes logged in main.cpp

## Phase 4: HEADLIGHTS Switch Testing

### Test 4.1: HEADLIGHTS Basic Operation
- [ ] Flip HEADLIGHTS switch ON
- [ ] See log: "HEADLIGHTS ON"
- [ ] `SN_Switches_AreHeadlightsOn()` returns TRUE
- [ ] `xr4_system_context.Headlights_On` is TRUE

- [ ] Flip HEADLIGHTS switch OFF
- [ ] See log: "HEADLIGHTS OFF"
- [ ] `SN_Switches_AreHeadlightsOn()` returns FALSE
- [ ] `xr4_system_context.Headlights_On` is FALSE

### Test 4.2: HEADLIGHTS Change Detection
- [ ] Flag `headlights_changed` is TRUE on change
- [ ] Flag cleared after reading

### Test 4.3: HEADLIGHTS Response Time
- [ ] Flip switch and observe Serial timing
- [ ] Response feels immediate (< 50ms perceived lag)
- [ ] No noticeable delay

## Phase 5: Rotary Encoder Testing

### Test 5.1: Encoder Rotation - Clockwise
- [ ] Rotate encoder clockwise
- [ ] Position increments (positive delta)
- [ ] Each click detected
- [ ] No missed steps

### Test 5.2: Encoder Rotation - Counter-Clockwise
- [ ] Rotate encoder counter-clockwise
- [ ] Position decrements (negative delta)
- [ ] Each click detected
- [ ] No missed steps

### Test 5.3: Encoder Delta Reading
```cpp
int8_t delta = SN_Switches_GetEncoderDelta();
Serial.printf("Delta: %d\n", delta);
```
- [ ] Returns -1 for CCW step
- [ ] Returns +1 for CW step
- [ ] Returns 0 when no movement
- [ ] Delta cleared after reading

### Test 5.4: Encoder Position Tracking
```cpp
int16_t pos = SN_Switches_GetEncoderPosition();
Serial.printf("Position: %d\n", pos);
```
- [ ] Position tracked accurately
- [ ] Can go negative
- [ ] Can exceed 100+

### Test 5.5: Encoder Reset
```cpp
SN_Switches_ResetEncoder();
```
- [ ] Position resets to 0
- [ ] See log: "Encoder position reset to 0"

### Test 5.6: Encoder Button
- [ ] Press rotary encoder button
- [ ] Flag `rotary_pressed` becomes TRUE
- [ ] Button state reflects press (LOW = pressed)
- [ ] Release button, state returns to FALSE

### Test 5.7: Fast Rotation
- [ ] Rotate encoder rapidly
- [ ] All steps captured
- [ ] No count errors
- [ ] Position remains accurate

## Phase 6: Integration Testing

### Test 6.1: System Context Updates
```cpp
Serial.printf("Armed: %d\n", xr4_system_context.Armed);
Serial.printf("E-Stop: %d\n", xr4_system_context.Emergency_Stop);
Serial.printf("Lights: %d\n", xr4_system_context.Headlights_On);
Serial.printf("Encoder: %d\n", xr4_system_context.Encoder_Pos);
```
- [ ] All values update correctly
- [ ] Values persist across loop iterations
- [ ] Ready for ESP-NOW transmission

### Test 6.2: Simultaneous Inputs
- [ ] Operate ARM, HEADLIGHTS, and encoder together
- [ ] All inputs processed correctly
- [ ] No interference between inputs
- [ ] All state changes logged

### Test 6.3: E-STOP Priority
- [ ] Trigger E-STOP while operating other switches
- [ ] E-STOP processes immediately
- [ ] Other operations halted
- [ ] Demonstrates interrupt priority

### Test 6.4: Main Loop Performance
```cpp
unsigned long loopTime = millis();
// ... main loop code ...
loopTime = millis() - loopTime;
Serial.printf("Loop time: %lu ms\n", loopTime);
```
- [ ] Loop time remains low (< 10ms typical)
- [ ] No significant overhead from switch handling

## Phase 7: Edge Cases

### Test 7.1: Rapid Switch Toggling
- [ ] Rapidly toggle ARM switch
- [ ] Debouncing prevents false triggers
- [ ] Only stable states registered

### Test 7.2: Encoder Overflow
- [ ] Rotate encoder 500+ steps in one direction
- [ ] Position tracking remains accurate
- [ ] No overflow errors (16-bit signed: -32768 to +32767)

### Test 7.3: Multiple Event Flags
- [ ] Change ARM, HEADLIGHTS, and press encoder button simultaneously
- [ ] All flags set correctly
- [ ] All flags cleared after reading

### Test 7.4: Long Duration Testing
- [ ] Run system for 10+ minutes
- [ ] All switches remain responsive
- [ ] No memory leaks
- [ ] No degradation in performance

## Phase 8: Safety Verification

### Test 8.1: E-STOP While Armed
- [ ] ARM the system
- [ ] Trigger E-STOP
- [ ] System immediately disarms/stops
- [ ] State transitions to XR4_STATE_EMERGENCY_STOP

### Test 8.2: Cannot ARM with E-STOP Active
- [ ] Trigger E-STOP first
- [ ] Try to ARM
- [ ] Arming prevented or overridden by E-STOP
- [ ] Safety maintained

### Test 8.3: E-STOP Recovery
- [ ] Trigger E-STOP
- [ ] Clear E-STOP
- [ ] System can be re-armed
- [ ] Full functionality restored

## Phase 9: ESP-NOW Transmission

### Test 9.1: Data Transmission to OBC
- [ ] Monitor ESP-NOW transmissions
- [ ] Switch states transmitted correctly
- [ ] Encoder position transmitted
- [ ] OBC receives and processes data

### Test 9.2: Transmission Timing
- [ ] Verify transmission rate
- [ ] No excessive delays
- [ ] Real-time response on OBC

## Phase 10: Documentation Verification

- [ ] README.md is complete and accurate
- [ ] QUICK_REFERENCE.md is helpful
- [ ] Examples compile and run
- [ ] Pin definitions match hardware
- [ ] All functions documented

## Test Results Summary

### Passed Tests
- [ ] All Phase 1 tests (Initialization)
- [ ] All Phase 2 tests (E-STOP) **CRITICAL**
- [ ] All Phase 3 tests (ARM)
- [ ] All Phase 4 tests (HEADLIGHTS)
- [ ] All Phase 5 tests (Encoder)
- [ ] All Phase 6 tests (Integration)
- [ ] All Phase 7 tests (Edge Cases)
- [ ] All Phase 8 tests (Safety) **CRITICAL**
- [ ] All Phase 9 tests (ESP-NOW)
- [ ] All Phase 10 tests (Documentation)

### Failed Tests
(List any failures here)

### Issues Found
(List any issues or concerns)

### Recommendations
(List any recommendations for improvements)

---

## Sign-Off

**Tester Name**: _________________  
**Date**: _________________  
**Result**: ☐ PASS ☐ FAIL ☐ CONDITIONAL  

**Notes**:

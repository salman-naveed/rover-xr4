# Hardware Wiring Diagram for SN_Switches Test

## Complete Wiring Schematic

```
                                    ESP32
                    ┌───────────────────────────────┐
                    │                               │
                    │          [USB Port]           │
                    │                               │
    ARM SWITCH      │  GPIO 4  ●───────────┐        │
    ┌─────┐         │                      │        │
    │  O  O─────────┼─────[ ]──────────────┘        │
    └─────┘    │    │                               │
               │    │                     3.3V ●────┼───┐
               └────┼─────[ GND ]                   │   │
                    │                               │  [10kΩ]
                    │                               │   │
  HEADLIGHTS        │  GPIO 19 ●───────────┐        │   │
    ┌─────┐         │                      │        │   │
    │  O  O─────────┼─────[ ]──────────────┴────────┼───┤
    └─────┘    │    │                               │   │
               │    │                               │  [10kΩ]
               └────┼─────[ GND ]                   │   │
                    │                               │   │
                    │                               │   │
   E-STOP (NC)      │  GPIO 14 ●───────────┐        │   │
    ┌─────┐         │                      │        │   │
    │  NC ●─────────┼─────[ ]──────────────┴────────┼───┘
    │     │         │                               │
    │  NO │         │                               │
    └─────┘    │    │                               │
               │    │                               │
               └────┼─────[ GND ]                   │
                    │                               │
                    │                               │
 ROTARY ENCODER     │                               │
    ┌─────┐         │                               │
    │ CLK ●─────────┼─────●  GPIO 32                │
    │  DT ●─────────┼─────●  GPIO 33                │
    │  SW ●─────────┼─────●  GPIO 23                │
    │  +  ●─────────┼─────●  3.3V (optional)        │
    │ GND ●─────────┼─────[ GND ]                   │
    └─────┘         │                               │
                    │                               │
                    │                               │
                    └───────────────────────────────┘

Legend:
  ●   Connection point
  [ ] Wire
  O   Switch terminal
  NC  Normally Closed contact
  NO  Normally Open contact
```

## Detailed Component Connections

### 1. ARM Switch (SPST - Single Pole Single Throw)

```
         ARM Switch
           ┌───┐
GPIO 4 ────┤   ├──── GND
           └───┘
             │
            10kΩ
             │
           3.3V
```

**Connections:**
- Terminal 1: GPIO 4
- Terminal 2: GND
- Pullup: 10kΩ resistor from GPIO 4 to 3.3V

**Logic:**
- Switch OPEN:   GPIO 4 = HIGH (pulled up to 3.3V)
- Switch CLOSED: GPIO 4 = LOW (connected to GND)

### 2. HEADLIGHTS Switch (SPST)

```
      HEADLIGHTS Switch
           ┌───┐
GPIO 19 ───┤   ├──── GND
           └───┘
             │
            10kΩ
             │
           3.3V
```

**Connections:**
- Terminal 1: GPIO 19
- Terminal 2: GND
- Pullup: 10kΩ resistor from GPIO 19 to 3.3V

**Logic:**
- Switch OPEN:   GPIO 19 = HIGH (pulled up to 3.3V)
- Switch CLOSED: GPIO 19 = LOW (connected to GND)

### 3. E-STOP Switch (NC - Normally Closed) ⚠️ CRITICAL

```
       E-STOP Switch (NC)
           ┌─────┐
           │  NC ●────── GPIO 14
GPIO 14 ───┤     │
           │  NO │ (not connected)
           └─────┘
             │
            GND
             
           10kΩ
             │
           3.3V ────────── GPIO 14
```

**Connections:**
- NC (Normally Closed) terminal: GPIO 14
- Common terminal: GND
- NO (Normally Open): Not used
- Pullup: 10kΩ resistor from GPIO 14 to 3.3V

**Logic (FAIL-SAFE):**
- Normal (closed):   GPIO 14 = LOW (NC contact connects to GND)
- Triggered (open):  GPIO 14 = HIGH (pulled up to 3.3V)
- Wire broken:       GPIO 14 = HIGH (TRIGGERS E-STOP) ← FAIL-SAFE!

⚠️ **IMPORTANT**: 
- Use NC (Normally Closed) terminal
- When switch is in resting/normal state, it should be CLOSED
- When pressed/activated, circuit OPENS

### 4. Rotary Encoder with Button

```
     Rotary Encoder
    ┌──────────────┐
    │  CLK  ●──────┼──── GPIO 32 (with internal pullup)
    │   DT  ●──────┼──── GPIO 33 (with internal pullup)
    │   SW  ●──────┼──── GPIO 23 (with internal pullup)
    │   +   ●──────┼──── 3.3V (optional - internal pullups used)
    │  GND  ●──────┼──── GND
    └──────────────┘
```

**Connections:**
- CLK pin: GPIO 32
- DT pin: GPIO 33
- SW (button) pin: GPIO 23
- VCC/+ pin: 3.3V (optional - can leave unconnected if encoder works)
- GND pin: GND

**Logic:**
- Internal pullups are enabled in code (INPUT_PULLUP)
- CLK and DT: Quadrature signals for rotation detection
- SW: Active LOW (pressed = LOW, released = HIGH)

## Breadboard Layout Example

```
                    ESP32 Dev Board
    ┌─────────────────────────────────────────┐
    │                                         │
    │  3.3V  GND   4   14   19   23  32  33  │
    │   ●    ●    ●    ●    ●    ●   ●   ●   │
    └───┬────┬────┬────┬────┬────┬───┬───┬───┘
        │    │    │    │    │    │   │   │
        │    │    │    │    │    │   │   └─────── Encoder DT
        │    │    │    │    │    │   └─────────── Encoder CLK
        │    │    │    │    │    └───────────── Encoder SW (Button)
        │    │    │    │    └────────────────── HEADLIGHTS Switch
        │    │    │    └─────────────────────── E-STOP NC
        │    │    └──────────────────────────── ARM Switch
        │    └───────────────────────────────── Common GND
        └────────────────────────────────────── Common 3.3V
```

## Component Shopping List

| Component | Quantity | Specification | Notes |
|-----------|----------|---------------|-------|
| ESP32 Dev Board | 1 | Any ESP32 | Must have GPIOs 4, 14, 19, 23, 32, 33 |
| SPST Toggle Switch | 2 | 6-pin panel mount | For ARM and HEADLIGHTS |
| NC Push Button | 1 | Normally Closed | For E-STOP (red mushroom style recommended) |
| Rotary Encoder | 1 | KY-040 or similar | With push button |
| 10kΩ Resistor | 3 | 1/4W | For pullups (ARM, HEADLIGHTS, E-STOP) |
| Breadboard | 1 | 830 points | For prototyping |
| Jumper Wires | ~15 | M-M or M-F | Various lengths |

## External Pullup Resistor Placement

**Why External Pullups?**
- More reliable than internal pullups
- Better noise immunity
- Consistent voltage levels

**ARM Switch Pullup:**
```
      3.3V
        │
       10kΩ
        │
   GPIO 4 ●───── To ARM Switch
        │
       GND (via switch)
```

**HEADLIGHTS Switch Pullup:**
```
      3.3V
        │
       10kΩ
        │
  GPIO 19 ●───── To HEADLIGHTS Switch
        │
       GND (via switch)
```

**E-STOP Pullup:**
```
      3.3V
        │
       10kΩ
        │
  GPIO 14 ●───── To E-STOP NC terminal
        │
       GND (via E-STOP when closed)
```

## Optional: Hardware RC Filter for E-STOP

For additional noise immunity (recommended for production):

```
      3.3V
        │
       10kΩ  (existing pullup)
        │
  GPIO 14 ●─────┬───── To E-STOP NC
        │       │
       10kΩ    100nF
        │       │
       GND     GND
```

**RC Time Constant:** τ = R × C = 10kΩ × 100nF = 1ms

This creates a low-pass filter that helps eliminate high-frequency noise.

## Testing Connections with Multimeter

### Test 1: Verify Pullups

1. **ARM Switch (GPIO 4)**
   - Switch OPEN: Measure GPIO 4 → should read ~3.3V
   - Switch CLOSED: Measure GPIO 4 → should read ~0V

2. **HEADLIGHTS Switch (GPIO 19)**
   - Switch OPEN: Measure GPIO 19 → should read ~3.3V
   - Switch CLOSED: Measure GPIO 19 → should read ~0V

3. **E-STOP (GPIO 14)**
   - Switch CLOSED (normal): Measure GPIO 14 → should read ~0V
   - Switch OPEN (triggered): Measure GPIO 14 → should read ~3.3V

### Test 2: Continuity Test

1. **E-STOP NC Contact**
   - Test between NC terminal and common
   - Should show continuity when button is NOT pressed
   - Should show open circuit when button IS pressed

2. **ARM/HEADLIGHTS Switches**
   - Should show continuity when switch is ON/CLOSED
   - Should show open circuit when switch is OFF/OPEN

## Common Wiring Mistakes

❌ **WRONG: Using NO instead of NC for E-STOP**
```
E-STOP NO ──── GPIO 14  ← WRONG! Not fail-safe!
```

✅ **CORRECT: Using NC for E-STOP**
```
E-STOP NC ──── GPIO 14  ← CORRECT! Fail-safe design
```

❌ **WRONG: Pullup to 5V**
```
5V ──── 10kΩ ──── GPIO  ← WRONG! ESP32 is 3.3V!
```

✅ **CORRECT: Pullup to 3.3V**
```
3.3V ──── 10kΩ ──── GPIO  ← CORRECT!
```

❌ **WRONG: No pullup resistor**
```
GPIO 4 ──── Switch ──── GND  (floating when open)
```

✅ **CORRECT: With pullup**
```
3.3V ──── 10kΩ ──── GPIO 4 ──── Switch ──── GND
```

## Safety Notes

⚠️ **E-STOP SAFETY**
1. E-STOP must be NC (Normally Closed) type
2. Always use external pullup resistor
3. Test fail-safe: disconnect wire should trigger E-STOP
4. Use red mushroom-style button for easy identification
5. Mount E-STOP in easily accessible location

⚠️ **ELECTRICAL SAFETY**
1. Never exceed 3.3V on any ESP32 GPIO
2. Use appropriate current-limiting resistors
3. Don't connect 5V directly to ESP32 pins
4. Ensure proper GND connection

⚠️ **ESD PROTECTION**
1. Touch grounded metal before handling ESP32
2. Work on anti-static mat if available
3. Avoid wearing synthetic clothing

## Verification Checklist

Before powering on:

- [ ] All pullup resistors are 10kΩ (not 10Ω or 100kΩ)
- [ ] Pullups connected to 3.3V (NOT 5V)
- [ ] E-STOP uses NC (Normally Closed) contact
- [ ] All switches connect to GND
- [ ] Encoder VCC connected to 3.3V or left unconnected
- [ ] Encoder GND connected to ESP32 GND
- [ ] No short circuits between power rails
- [ ] Correct GPIO pins used (match code)
- [ ] All connections secure

After powering on:

- [ ] ESP32 boots successfully
- [ ] Serial output appears at 115200 baud
- [ ] Initial states shown correctly
- [ ] All switches respond to input
- [ ] E-STOP triggers on wire disconnect

---

**Document Version:** 1.0  
**Last Updated:** December 15, 2025  
**Compatible With:** SN_Switches_Standalone_Test.ino

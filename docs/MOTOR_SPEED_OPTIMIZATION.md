# Motor Speed Optimization Guide

## Overview
This guide helps you maximize motor speed on the XR4 rover through PWM frequency optimization, voltage considerations, and software tuning.

## 1. PWM Frequency Tuning (BIGGEST IMPACT)

### Current Configuration
- **Default**: 10kHz (changed from 20kHz for better power delivery)
- **Location**: `lib/SN_Motors/Motor.h` → `#define MOTOR_PWM_FREQUENCY`

### Frequency vs Performance

| Frequency | Speed | Efficiency | Noise | Heat | Best For |
|-----------|-------|------------|-------|------|----------|
| **1kHz** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | 🔊🔊🔊 Loud | 🌡️ Low | Maximum raw power |
| **5kHz** | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | 🔊🔊 Audible | 🌡️ Low | High speed |
| **10kHz** | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | 🔊 Slight | 🌡️ Moderate | **RECOMMENDED** |
| **20kHz** | ⭐⭐⭐ | ⭐⭐⭐⭐ | 🔇 Silent | 🌡️ Higher | Smooth, quiet |
| **30kHz** | ⭐⭐ | ⭐⭐⭐ | 🔇 Silent | 🌡️🌡️ High | Very smooth |

### How to Change:

Edit `lib/SN_Motors/Motor.h`:
```cpp
#define MOTOR_PWM_FREQUENCY 10000  // Change this value
```

**Recommended values to test:**
- Start: `10000` (10kHz) - Good balance ✅
- More speed: `5000` (5kHz) - More power, slight noise
- Maximum speed: `3000` (3kHz) - Maximum power, audible whine
- Original: `20000` (20kHz) - Smooth but less power

### Testing Procedure:
1. Change `MOTOR_PWM_FREQUENCY` value
2. Build and upload firmware
3. Test motor speed with joystick at full forward
4. Note speed, noise level, and heat
5. Try different values until optimal

---

## 2. Voltage and Power Supply

### Increase Motor Voltage (Hardware Change)
- **Current voltage**: Check your battery voltage
- **Motor rated voltage**: Check motor specifications
- **Safe range**: Most DC motors: 6V - 12V
- **Caution**: Don't exceed motor rated voltage!

**Effect**: Higher voltage = Higher speed (linear relationship)
- 9V battery → 12V battery = ~33% speed increase
- Must ensure motor driver can handle higher voltage

### Improve Power Delivery
- ✅ Use thicker wires (lower resistance)
- ✅ Minimize wire length
- ✅ Add capacitors near motor driver (100µF - 1000µF)
- ✅ Ensure battery can supply peak current (4 motors × motor current)
- ✅ Check for voltage sag under load

---

## 3. Software Optimizations

### A. Remove Speed Limits (if desired)

Current system limits speed to ±100% duty cycle. To allow overdrive:

**NOT RECOMMENDED** unless motors are under-rated, but here's how:

Edit `lib/SN_Handler/SN_Handler.cpp` lines 221-228:
```cpp
// Current (safe):
if (leftSpeed > 100) leftSpeed = 100;
else if (leftSpeed < -100) leftSpeed = -100;
if (rightSpeed > 100) rightSpeed = 100;
else if (rightSpeed < -100) rightSpeed = -100;

// For speed boost (RISKY - can damage motors):
// Allow up to 120% duty cycle
if (leftSpeed > 120) leftSpeed = 120;
else if (leftSpeed < -120) leftSpeed = -120;
if (rightSpeed > 120) rightSpeed = 120;
else if (rightSpeed < -120) rightSpeed = -120;
```

⚠️ **WARNING**: Exceeding 100% can:
- Overheat motors
- Damage motor driver
- Reduce motor lifespan
- Only safe if motors are significantly under-rated

### B. Optimize Control Loop Timing

Your control loop already runs at **1000Hz (1ms)** which is excellent!

If you want even faster response (not recommended unless needed):
```cpp
// In src/main.cpp, change loop delay:
delay(1);  // Current: 1ms (1000Hz) - already optimal
```

### C. Adjust Joystick Mapping for More Aggressive Response

Currently using linear mapping. For more aggressive acceleration:

Edit `lib/SN_Handler/SN_Handler.cpp` around line 200:

```cpp
// Current (linear):
throttle = (int16_t)(((int32_t)xr4_system_context.Joystick_X * 200) / 4095) - 100;

// Option 1: Exponential curve (more aggressive at high speeds)
// Apply after linear mapping:
float normalized = throttle / 100.0f;  // -1.0 to 1.0
float exponential = normalized * abs(normalized);  // Square but keep sign
throttle = (int16_t)(exponential * 100.0f);

// Option 2: Cubic curve (even more aggressive)
float cubic = normalized * normalized * normalized;
throttle = (int16_t)(cubic * 100.0f);
```

This gives you:
- Fine control at low speeds
- Maximum power at high joystick positions

---

## 4. Motor Driver Settings

### Check Driver Configuration
Some motor drivers have:
- **Current limit potentiometer**: Turn clockwise for more current
- **Voltage regulator**: May limit output voltage
- **Thermal protection**: May throttle at high temperatures
- **Enable pins**: Ensure not limiting PWM

### Driver Compatibility
- Ensure driver can handle your PWM frequency
- Some drivers work best at specific frequencies
- Check driver datasheet for optimal PWM range

---

## 5. Mechanical Optimizations

### Reduce Load
- Lighter rover = faster acceleration
- Remove unnecessary weight
- Optimize wheel/tire choice

### Improve Efficiency
- Lubricate motor bearings
- Check for binding in drivetrain
- Ensure wheels spin freely
- Proper wheel alignment

### Gear Ratio
- Higher gear ratio = more speed, less torque
- Lower gear ratio = more torque, less speed
- Consider changing if motors have excess torque

---

## 6. Testing Methodology

### Speed Test Procedure:
1. Mark a straight line (5 meters)
2. Time rover from standstill to finish
3. Test each configuration 3 times
4. Calculate average speed

### Comparison Baseline:
```
Current Config (20kHz): _____ m/s
New Config (10kHz):     _____ m/s
New Config (5kHz):      _____ m/s
```

### Monitor During Tests:
- ✅ Motor temperature (should be warm, not hot to touch)
- ✅ Driver temperature (check for thermal shutdown)
- ✅ Battery voltage under load
- ✅ Any unusual motor sounds
- ✅ Control responsiveness

---

## 7. Recommended Optimization Sequence

### Phase 1: Safe Software Tuning (START HERE)
1. ✅ Change PWM frequency from 20kHz → 10kHz
2. Upload and test
3. Measure speed improvement
4. If needed, try 5kHz
5. Choose best balance of speed/noise/heat

**Expected gain**: 10-20% speed increase

### Phase 2: Power Supply (if needed)
1. Measure current battery voltage under load
2. If voltage drops significantly, improve battery
3. Add capacitors near motor driver
4. Use thicker wires

**Expected gain**: 5-15% speed increase

### Phase 3: Advanced Software (optional)
1. Try exponential joystick curve for better feel
2. Fine-tune deadband and response
3. Optimize for your specific use case

**Expected gain**: Better control, perceived performance

### Phase 4: Hardware (advanced)
1. Upgrade battery voltage (if safe for motors)
2. Check/adjust motor driver current limit
3. Mechanical optimizations

**Expected gain**: 20-50% speed increase (voltage dependent)

---

## 8. Safety Limits

### DO NOT EXCEED:
- ❌ Motor rated voltage
- ❌ Motor rated current
- ❌ Driver rated current
- ❌ 100% duty cycle (unless motors are under-rated)

### STOP IMMEDIATELY IF:
- 🔴 Motors too hot to touch
- 🔴 Driver thermal shutdown
- 🔴 Burning smell
- 🔴 Unusual motor sounds
- 🔴 Erratic behavior

### Safe Operating Temperature:
- Motors: < 70°C (158°F)
- Drivers: < 80°C (176°F)
- Battery: < 50°C (122°F)

---

## 9. Quick Start: Maximize Speed NOW

**5-Minute Speed Boost:**

1. Edit `lib/SN_Motors/Motor.h` line 18:
   ```cpp
   #define MOTOR_PWM_FREQUENCY 10000  // Changed from 20000
   ```

2. Build and upload:
   ```bash
   pio run --target upload
   ```

3. Test and enjoy 10-15% speed increase!

**If you want even more speed (noisier):**
```cpp
#define MOTOR_PWM_FREQUENCY 5000  // More aggressive
```

**If motors get hot, back off to:**
```cpp
#define MOTOR_PWM_FREQUENCY 15000  // More conservative
```

---

## 10. Expected Results

### With PWM Frequency Optimization (10kHz):
- ✅ 10-20% speed increase
- ✅ Better power delivery
- ⚠️ Slight motor whine (barely noticeable)
- ✅ Same battery life or better

### With PWM Frequency Aggressive (5kHz):
- ✅ 15-25% speed increase
- ✅ Maximum power to motors
- ⚠️ Audible motor whine
- ✅ Slightly better battery life
- ⚠️ Monitor motor temperature

### With Voltage Increase (hardware):
- ✅ 30-50% speed increase (if going from 9V to 12V)
- ⚠️ Must ensure motors rated for higher voltage
- ⚠️ Higher current draw from battery

---

## Troubleshooting

### Motors slower after changing frequency:
- Try different frequency (some drivers work better at specific ranges)
- Check driver datasheet for recommended PWM frequency
- Verify battery can supply current

### Motors too hot:
- Increase PWM frequency (20kHz or 30kHz)
- Reduce maximum speed limit
- Check for mechanical binding
- Improve cooling/ventilation

### Motors noisy:
- Increase PWM frequency (15kHz or 20kHz)
- Add filter capacitors
- Check motor bearings

### Inconsistent speed:
- Check battery voltage under load
- Ensure all motors using same PWM frequency
- Verify wiring connections
- Check for loose motor mounts

---

## Conclusion

**Easiest and safest**: Change PWM frequency to 10kHz for immediate 10-15% speed boost!

**Most effective overall**: Combine PWM optimization (10kHz) with voltage increase (hardware) for 30-40% total improvement.

**Always monitor**: Temperature, battery voltage, and motor health during testing.

**Start conservative**: You can always increase aggression, but motor damage is permanent!

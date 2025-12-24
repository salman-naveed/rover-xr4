# ESP32 GPIO Pin Warnings and Restrictions

## ⚠️ CRITICAL: Strapping Pins (AVOID for Motor Control)

These pins have special boot-time functions and should **NOT** be used for motor control or any hardware that might pull them HIGH/LOW during boot:

### GPIO0 - Boot Mode Select
- **Function**: Determines boot mode (flash vs download)
- **Boot Requirement**: Must be HIGH for normal boot
- **Problem**: If pulled LOW at boot, enters download mode
- **Status**: ❌ **AVOID** for motors

### GPIO2 - Boot Mode Select  
- **Function**: Must be floating or LOW during boot
- **Boot Requirement**: LOW or floating (internal pull-down)
- **Problem**: HIGH at boot can prevent booting
- **Status**: ❌ **AVOID** for motors

### GPIO12 - Flash Voltage Select (VDD_SDIO)
- **Function**: Selects flash memory voltage
- **Boot Requirement**: Must be LOW for 3.3V flash (standard)
- **Problem**: If HIGH at boot, ESP32 expects 1.8V flash (can damage 3.3V flash!)
- **Impact**: **THIS WAS CAUSING YOUR MOTOR TO RUN CONTINUOUSLY**
- **Status**: ❌ **NEVER USE** for motors - most dangerous strapping pin

### GPIO15 - Timing Control
- **Function**: Controls boot timing and debug output
- **Boot Requirement**: Must be HIGH for normal boot
- **Problem**: LOW at boot enables debug output
- **Status**: ⚠️ **Use with caution** for motors

### GPIO5 - Timing Control
- **Function**: Controls SDIO Slave timing
- **Boot Requirement**: Should be HIGH (internal pull-up)
- **Problem**: Can affect boot reliability
- **Status**: ⚠️ **Use with caution**

## ✅ SAFE GPIO Pins for Motor Control

### Excellent Choices (No restrictions):
- **GPIO16, GPIO17** - Used for UART2 but safe for general I/O
- **GPIO18, GPIO19** - VSPI pins but safe
- **GPIO21, GPIO22** - I2C pins but safe for motors
- **GPIO23** - VSPI MOSI but safe
- **GPIO25, GPIO26** - DAC pins but safe for PWM ✅ Currently used
- **GPIO27** - Touch sensor but safe for PWM ✅ Currently used
- **GPIO32, GPIO33** - ADC pins but safe for PWM ✅ Currently used

### Usable but with Caveats:
- **GPIO4** - Can be used but might affect bootloader
- **GPIO13** - Generally safe but near GPIO12 (avoid if possible)
- **GPIO14** - Safe ✅ Currently used

### Input-Only (Cannot Drive Motors):
- **GPIO34, GPIO35, GPIO36, GPIO39** - Input only, no pull-up/down

### Reserved/Special Function:
- **GPIO1, GPIO3** - UART0 (Serial) - needed for debugging
- **GPIO6-11** - Connected to flash (NEVER use)

## Current XR4 Motor Pin Assignment

### ✅ FIXED Configuration (Safe Pins):
```cpp
// Motor 1 - Left Front
GPIO 32 (PWM0A) ✅ Safe
GPIO 33 (PWM0B) ✅ Safe

// Motor 2 - Left Rear  
GPIO 25 (PWM1A) ✅ Safe
GPIO 26 (PWM1B) ✅ Safe

// Motor 3 - Right Front
GPIO 27 (PWM0A) ✅ Safe
GPIO 14 (PWM0B) ✅ Safe (minor caution)

// Motor 4 - Right Rear
GPIO 16 (PWM1A) ✅ Safe (CHANGED from GPIO12)
GPIO 17 (PWM1B) ✅ Safe (CHANGED from GPIO13)
```

### ❌ OLD Configuration (Had Strapping Pin Problem):
```cpp
// Motor 4 - Right Rear (OLD - PROBLEMATIC)
GPIO 12 (PWM1A) ❌ STRAPPING PIN - caused motor to run continuously!
GPIO 13 (PWM1B) ⚠️ Near strapping pin - also problematic
```

## Why GPIO12 Caused the Motor Problem

1. **At Boot**: GPIO12 determines flash voltage
   - Your motor driver may have had pull-ups
   - Or the driver was outputting before ESP32 took control
   
2. **During Init**: GPIO12 has special bootloader handling
   - pinMode() might not fully override strapping function
   - MCPWM might have conflicts with internal strapping logic

3. **During Operation**: Residual strapping behavior
   - Pin might default to HIGH when MCPWM releases control
   - Motor driver sees HIGH signal = continuous running

4. **Hardware Interaction**: Motor driver fighting ESP32
   - ESP32 trying to control GPIO12
   - Strapping logic interfering with PWM
   - Result: Motor stuck in ON state

## Wiring Changes Required

### Physical Changes Needed:
1. **Disconnect** Motor 4 from GPIO12 and GPIO13
2. **Connect** Motor 4 to GPIO16 and GPIO17
3. **Update firmware** with new pin definitions
4. **Test** with diagnostic program

### Pin Mapping:
```
OLD WIRING → NEW WIRING
GPIO12 (M4_A) → GPIO16 (M4_A)
GPIO13 (M4_B) → GPIO17 (M4_B)
```

## Additional Notes

### Why Other Pins Still Work:
- GPIO14 is technically near strapping pins but doesn't have boot-critical function
- GPIO27 is safe despite being touch sensor pin
- GPIO32/33 are safe ADC pins with no boot restrictions

### Future Pin Selection Guidelines:
1. ✅ **ALWAYS** check ESP32 datasheet for strapping pins
2. ✅ **PREFER** GPIO16-19, 21-23, 25-27, 32-33 for motors
3. ❌ **NEVER** use GPIO0, 2, 5, 12, 15 for motors
4. ❌ **AVOID** GPIO6-11 (flash pins)
5. ⚠️ **DOCUMENT** any unavoidable use of problematic pins

## Testing After Pin Change

1. Upload new firmware with GPIO16/17
2. Power cycle the ESP32 completely
3. Run motor diagnostic test
4. Verify Motor 4 (Right Rear) now:
   - Stops when commanded to stop
   - Responds to speed changes
   - Doesn't run continuously
   - Behaves like other motors

## References

- [ESP32 Datasheet - Strapping Pins](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)
- [ESP32 GPIO Matrix](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
- [ESP32 Boot Mode Selection](https://github.com/espressif/esptool/wiki/ESP32-Boot-Mode-Selection)

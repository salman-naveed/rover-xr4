# CTU Available GPIO Pins Reference

**Date**: December 17, 2025  
**Purpose**: GPIO pin allocation analysis for adding 2-position switch (2 inputs) + 3 LEDs (3 outputs)

---

## 📊 Current CTU GPIO Usage

### **GPIOs Currently In Use:**

| GPIO | Function | Type | Module | Notes |
|------|----------|------|--------|-------|
| **2** | RGB LED Strip | Output | SN_StatusPanel | WS2812 control pin |
| **4** | ARM Switch | Input | SN_Switches | External pullup |
| **14** | E-STOP Switch | Input | SN_Switches | NC switch, **SAFETY CRITICAL** |
| **19** | HEADLIGHTS Switch | Input | SN_Switches | External pullup |
| **21** | I2C SDA | I2C | LCD | 20x4 LCD display |
| **22** | I2C SCL | I2C | LCD | 20x4 LCD display |
| **23** | Rotary Button | Input | SN_Switches | Encoder push button |
| **25** | Button A | Input | SN_Common/SN_Joystick | Legacy button (initialized) |
| **26** | Button B | Input | SN_Common/SN_Joystick | Legacy button (initialized) |
| **27** | Button C | Input | SN_Common/SN_Joystick | Legacy button (initialized) |
| **32** | Rotary Encoder CLK | Input | SN_Switches | Quadrature A |
| **33** | Rotary Encoder DT | Input | SN_Switches | Quadrature B |
| **36** | Joystick Y (A0/SVP) | ADC Input | SN_Joystick | Analog only, input only |
| **39** | Joystick X (A3/SVN) | ADC Input | SN_Joystick | Analog only, input only |

**Total Used: 14 GPIOs**

---

## ✅ Available GPIOs for Digital I/O

### **Recommended Safe Pins (No Boot Restrictions):**

| GPIO | Type | Quality | Use Case | Boot Behavior |
|------|------|---------|----------|---------------|
| **5** | Digital I/O | ⭐⭐⭐ EXCELLENT | Input/Output | Strapping pin, but stable HIGH during boot |
| **13** | Digital I/O | ⭐⭐⭐ EXCELLENT | Input/Output | Safe, no restrictions |
| **16** | Digital I/O | ⭐⭐⭐ EXCELLENT | Input/Output | Safe, commonly used for I/O |
| **17** | Digital I/O | ⭐⭐⭐ EXCELLENT | Input/Output | Safe, commonly used for I/O |
| **18** | Digital I/O | ⭐⭐⭐ EXCELLENT | Input/Output | Safe, commonly used for SPI/I/O |

### **Usable with Caution:**

| GPIO | Type | Quality | Caution | Boot Requirement |
|------|------|---------|---------|------------------|
| **12** | Digital I/O | ⭐⭐ CAUTION | Must be LOW during boot | Has internal pulldown |
| **15** | Digital I/O | ⭐⭐ CAUTION | Must be LOW during boot | Has pullup, debug output |

---

## ❌ GPIOs to AVOID

| GPIO | Why Avoid |
|------|-----------|
| **0** | Boot button - must be HIGH during boot, used for programming |
| **1** | UART0 TX (Serial Monitor) |
| **3** | UART0 RX (Serial Monitor) |
| **6-11** | Connected to SPI flash - **NEVER USE** (will brick board) |
| **34** | Input only, no pullup/pulldown |
| **35** | Input only, no pullup/pulldown |

---

## 🎯 RECOMMENDED PIN ASSIGNMENT

### **For Your Application (2-Position Switch + 3 LEDs):**

#### **Option 1: Best Pins (Recommended)**

```cpp
// File: lib/SN_Common/SN_GPIO_Definitions.h or new header

// 2-Position Switch (Digital Inputs with pullup)
#define SWITCH_POSITION_1_PIN  5   // Position 1 detection
#define SWITCH_POSITION_2_PIN  18  // Position 2 detection

// 3 Status LEDs (Digital Outputs)
#define STATUS_LED_PIN         16  // LED 1 - Status indicator
#define MODE_LED_PIN           17  // LED 2 - Mode indicator  
#define WARNING_LED_PIN        13  // LED 3 - Warning/Error indicator
```

**Wiring:**
- **Switch**: Connect each position to respective GPIO, switches pull to GND when active
- **LEDs**: GPIO → 220Ω resistor → LED anode (+), cathode (-) → GND

**Code Example:**
```cpp
// Initialization
pinMode(SWITCH_POSITION_1_PIN, INPUT_PULLUP);
pinMode(SWITCH_POSITION_2_PIN, INPUT_PULLUP);
pinMode(STATUS_LED_PIN, OUTPUT);
pinMode(MODE_LED_PIN, OUTPUT);
pinMode(WARNING_LED_PIN, OUTPUT);

// Reading 2-position switch
bool pos1_active = !digitalRead(SWITCH_POSITION_1_PIN);  // Active LOW
bool pos2_active = !digitalRead(SWITCH_POSITION_2_PIN);  // Active LOW

// LED control
digitalWrite(STATUS_LED_PIN, HIGH);   // Turn on
digitalWrite(MODE_LED_PIN, LOW);      // Turn off
```

---

#### **Option 2: Reclaim Legacy Buttons (If Not Used)**

**IF** you don't have physical buttons connected to GPIO 25, 26, 27, you can repurpose them:

1. **Remove from `SN_Joystick.cpp`** (lines 71-73, 226-228)
2. **Use the freed pins:**

```cpp
// 2-Position Switch
#define SWITCH_POSITION_1_PIN  25
#define SWITCH_POSITION_2_PIN  26

// 3 LEDs
#define LED1_PIN               27
#define LED2_PIN               5
#define LED3_PIN               18
```

**Note**: This requires code modification to remove Button A/B/C initialization from SN_Joystick module.

---

## 🔌 Pin Characteristics Summary

### **GPIO 5** (Strapping Pin)
- **Boot Behavior**: Must be HIGH for normal boot (has pullup)
- **Safe Use**: Yes, pulls HIGH by default
- **Best For**: Digital input/output, safe for switches

### **GPIO 13**
- **Boot Behavior**: No restrictions
- **Safe Use**: Yes
- **Best For**: Any digital I/O, LEDs, switches

### **GPIO 16 & 17**
- **Boot Behavior**: No restrictions
- **Safe Use**: Yes
- **Best For**: Any digital I/O, commonly used for UART2, safe for LEDs

### **GPIO 18**
- **Boot Behavior**: No restrictions
- **Safe Use**: Yes
- **Best For**: Any digital I/O, SPI SCK alternative, safe for switches/LEDs

---

## 📝 Integration Steps

### **Step 1: Add Pin Definitions**
Create or update header file:
```cpp
// File: lib/SN_Common/SN_GPIO_Definitions.h

// 2-Position Mode Switch
#define SWITCH_POSITION_1_PIN  5   
#define SWITCH_POSITION_2_PIN  18  

// Status LEDs
#define STATUS_LED_PIN         16  
#define MODE_LED_PIN           17  
#define WARNING_LED_PIN        13  
```

### **Step 2: Initialize in Setup**
```cpp
// In your initialization function
void SN_CustomIO_Init() {
    // Configure switch inputs with pullups
    pinMode(SWITCH_POSITION_1_PIN, INPUT_PULLUP);
    pinMode(SWITCH_POSITION_2_PIN, INPUT_PULLUP);
    
    // Configure LED outputs
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(MODE_LED_PIN, OUTPUT);
    pinMode(WARNING_LED_PIN, OUTPUT);
    
    // Initial LED states
    digitalWrite(STATUS_LED_PIN, LOW);
    digitalWrite(MODE_LED_PIN, LOW);
    digitalWrite(WARNING_LED_PIN, LOW);
}
```

### **Step 3: Read and Control**
```cpp
// Read 2-position switch (active LOW with pullup)
bool position1 = !digitalRead(SWITCH_POSITION_1_PIN);
bool position2 = !digitalRead(SWITCH_POSITION_2_PIN);

// Determine mode (mutually exclusive positions)
if (position1) {
    // Switch in position 1
    digitalWrite(MODE_LED_PIN, HIGH);
} else if (position2) {
    // Switch in position 2
    digitalWrite(MODE_LED_PIN, LOW);
}

// Control status LED
digitalWrite(STATUS_LED_PIN, system_ready ? HIGH : LOW);

// Blink warning LED
static uint32_t last_blink = 0;
if (millis() - last_blink > 500) {
    digitalWrite(WARNING_LED_PIN, !digitalRead(WARNING_LED_PIN));
    last_blink = millis();
}
```

---

## 🛡️ Safety Notes

1. **E-STOP Priority**: Your E-STOP (GPIO 14) must always have highest priority
2. **Boot Modes**: Avoid GPIO 0, 2, 12, 15 during boot (can cause boot failures)
3. **Flash Pins**: NEVER use GPIO 6-11 (connected to flash memory)
4. **Input-Only**: GPIO 34, 35, 36, 39 cannot be outputs
5. **Current Limits**: ESP32 GPIO max 40mA total, 12mA per pin (use resistors for LEDs)

---

## 🔗 Related Documentation

- [SN_Switches Implementation](../lib/SN_Switches/README.md)
- [GPIO Definitions](../lib/SN_Common/SN_GPIO_Definitions.h)
- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)

---

## 📊 Quick Reference Table

| Need | Recommended Pins | Alternative |
|------|------------------|-------------|
| **2 Digital Inputs** | GPIO 5, 18 | GPIO 25, 26 (if buttons removed) |
| **3 Digital Outputs** | GPIO 16, 17, 13 | GPIO 27, 5, 18 (if buttons removed) |
| **SPI (future)** | GPIO 18 (SCK), 19 (MISO), 23 (MOSI) | Used by switches currently |
| **UART (future)** | GPIO 16 (RX), 17 (TX) | UART2 alternative |

---

## ✅ Final Recommendation

**Use these 5 pins for maximum safety and compatibility:**

```cpp
GPIO  5  - Switch Position 1 Input
GPIO 18  - Switch Position 2 Input
GPIO 16  - LED 1 Output (Status)
GPIO 17  - LED 2 Output (Mode)
GPIO 13  - LED 3 Output (Warning)
```

**Why**: All pins are safe, have no boot restrictions, and are not used by any existing CTU modules.

---

**Last Updated**: December 17, 2025  
**Verified Against**: CTU Firmware v1.0 (ESP32 DevKit)

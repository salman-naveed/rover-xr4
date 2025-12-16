# XR-4 Rover Documentation

Complete documentation for the XR-4 Rover Control & Telemetry Unit (CTU) and On-Board Computer (OBC) firmware.

## 📚 Documentation Index

### System Architecture & Operation

- **[STARTUP_SEQUENCE.md](STARTUP_SEQUENCE.md)** - Complete CTU startup sequence, state machine, and timing analysis
- **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)** - Joystick fixes implementation summary
- **[JOYSTICK_ANALYSIS.md](JOYSTICK_ANALYSIS.md)** - Comprehensive joystick workflow analysis and optimization

### Component-Specific Documentation

#### LCD Display System
Location: `../lib/SN_LCD/`
- **[LCD_INTEGRATION_GUIDE.md](../lib/SN_LCD/LCD_INTEGRATION_GUIDE.md)** - How to integrate and use the LCD system
- **[LCD_PAGE_LAYOUTS.md](../lib/SN_LCD/LCD_PAGE_LAYOUTS.md)** - Detailed layout of all 5 LCD pages
- **[LCD_QUICK_REFERENCE.md](../lib/SN_LCD/LCD_QUICK_REFERENCE.md)** - Quick API reference
- **[LCD_TESTING_GUIDE.md](../lib/SN_LCD/LCD_TESTING_GUIDE.md)** - Testing procedures
- **[README.md](../lib/SN_LCD/README.md)** - LCD module overview

#### Switch System
Location: `../lib/SN_Switches/`
- **[README.md](../lib/SN_Switches/README.md)** - Switch system overview and architecture
- **[IMPLEMENTATION_SUMMARY.md](../lib/SN_Switches/IMPLEMENTATION_SUMMARY.md)** - Implementation details
- **[QUICK_REFERENCE.md](../lib/SN_Switches/QUICK_REFERENCE.md)** - API quick reference
- **[TESTING_CHECKLIST.md](../lib/SN_Switches/TESTING_CHECKLIST.md)** - Verification procedures
- **[WIRING_DIAGRAM.md](../lib/SN_Switches/WIRING_DIAGRAM.md)** - Hardware wiring guide

#### Joystick System
Location: `../lib/SN_Joystick/`
- **[JOYSTICK_ENHANCEMENTS_GUIDE.md](../lib/SN_Joystick/JOYSTICK_ENHANCEMENTS_GUIDE.md)** - Calibration, averaging, and enhancements

### Testing & Development

#### OBC Simulator
Location: `../test/OBC_Simulator_Test/`
- **[README.md](../test/OBC_Simulator_Test/README.md)** - OBC simulator setup and usage guide
- **[OBC_Simulator_Test.ino](../test/OBC_Simulator_Test/OBC_Simulator_Test.ino)** - Arduino sketch for ESP32

## 🚀 Quick Start Guides

### For CTU Development
1. Read [STARTUP_SEQUENCE.md](STARTUP_SEQUENCE.md) to understand system initialization
2. Check [LCD_INTEGRATION_GUIDE.md](../lib/SN_LCD/LCD_INTEGRATION_GUIDE.md) for display implementation
3. Review [Switch System README](../lib/SN_Switches/README.md) for input handling

### For Testing
1. Set up [OBC Simulator](../test/OBC_Simulator_Test/README.md) on a spare ESP32
2. Follow [LCD_TESTING_GUIDE.md](../lib/SN_LCD/LCD_TESTING_GUIDE.md) for display verification
3. Use [Switch Testing Checklist](../lib/SN_Switches/TESTING_CHECKLIST.md) for hardware validation

### For Troubleshooting
1. Check [STARTUP_SEQUENCE.md - Troubleshooting](STARTUP_SEQUENCE.md#troubleshooting) for common issues
2. Review Serial Monitor output against expected logs in documentation
3. Verify hardware wiring using [WIRING_DIAGRAM.md](../lib/SN_Switches/WIRING_DIAGRAM.md)

## 📖 Documentation Standards

All documentation follows these conventions:

### File Naming
- **ALL_CAPS_WITH_UNDERSCORES.md** - Major documentation (guides, analysis)
- **Title_Case_With_Underscores.md** - Component-specific docs
- **README.md** - Module overviews and entry points

### Content Structure
1. **Overview** - Brief description and purpose
2. **Features/Specifications** - What it does
3. **Implementation Details** - How it works
4. **Usage/API** - How to use it
5. **Examples** - Code samples
6. **Troubleshooting** - Common issues and solutions
7. **References** - Related documentation

### Code Examples
All code examples include:
- Complete context (includes, declarations)
- Inline comments explaining key steps
- Expected output where applicable

## 🔧 System Overview

### Hardware Architecture
```
CTU (Control & Telemetry Unit)
├── ESP32 Microcontroller
├── 20x4 LCD Display (I2C @ 0x27)
├── Analog Joystick (GPIO 36, 39)
├── Rotary Encoder (GPIO 32, 33, 23)
├── Switches:
│   ├── E-STOP (GPIO 14, NC)
│   ├── ARM (GPIO 4)
│   └── Headlights (GPIO 19)
└── Status LED Panel

OBC (On-Board Computer)
├── ESP32 Microcontroller
├── Dual Motors (MCPWM)
├── GPS Module (Serial)
├── IMU Sensor (I2C)
└── Status LED Panel

Communication: ESP-NOW (2.4GHz)
```

### Software Architecture
```
CTU Firmware
├── main.cpp - State machine & initialization
├── SN_Handler - Main control loop
├── SN_ESPNOW - ESP-NOW communication
├── SN_LCD - Multi-page display system
├── SN_Switches - Interrupt-driven inputs
├── SN_Joystick - ADC with calibration
└── SN_Common - Shared data structures

OBC Firmware
├── main.cpp - State machine & initialization
├── SN_Handler - Control execution
├── SN_ESPNOW - ESP-NOW communication
├── SN_Motors - MCPWM motor control
├── SN_GPS - GPS parsing
└── SN_Sensors - IMU reading
```

## 📊 Key Specifications

### Performance Metrics
- **CTU Loop Rate:** ~100 Hz (10ms/cycle)
- **LCD Update Rate:** 10 Hz (100ms, configurable)
- **ESP-NOW Latency:** <10ms typical
- **Switch Debounce:** 50ms
- **Joystick ADC:** 12-bit (0-4095), 4-sample averaging

### State Machine States
1. `XR4_STATE_JUST_POWERED_ON` (0) - Initial power-up
2. `XR4_STATE_INITIALIZED` (1) - Hardware ready
3. `XR4_STATE_COMMS_CONFIG` (2) - ESP-NOW setup
4. `XR4_STATE_WAITING_FOR_ARM` (3) - Ready, waiting for ARM
5. `XR4_STATE_ARMED` (4) - Operational mode
6. `XR4_STATE_ERROR` (5) - Error condition
7. `XR4_STATE_EMERGENCY_STOP` (6) - E-STOP active
8. `XR4_STATE_REBOOT` (8) - System restart

## 🛠️ Development Tools

### Required Software
- PlatformIO (recommended) or Arduino IDE
- ESP32 Arduino Core 2.x or 3.x
- Git for version control

### Required Hardware (for testing)
- 2x ESP32 development boards
- USB cables
- CTU components (LCD, joystick, switches)
- OBC components (motors, sensors) OR OBC Simulator

### Debugging Tools
- Serial Monitor (115200 baud)
- Logic analyzer (optional, for I2C/SPI debugging)
- Multimeter for hardware verification

## 📝 Contributing

When adding new documentation:
1. Place in appropriate location (`docs/` or component folder)
2. Update this index
3. Follow naming and structure conventions
4. Include practical examples
5. Add troubleshooting section

## 📞 Support & Resources

### Serial Monitor Diagnostic Codes
See [STARTUP_SEQUENCE.md](STARTUP_SEQUENCE.md#serial-monitor-diagnostic-output) for expected log output.

### Hardware Pinout References
- [Switch Wiring Diagram](../lib/SN_Switches/WIRING_DIAGRAM.md)
- [GPIO Definitions](../lib/SN_Common/SN_GPIO_Definitions.h)

### Communication Protocol
- [ESP-NOW Data Structures](../lib/SN_ESPNOW/SN_ESPNOW.h)
- [Telecommand Format](../lib/SN_Handler/XR4_RCU_Telecommands.h)

---

**Last Updated:** December 15, 2025  
**Firmware Version:** Compatible with ESP32 Arduino Core 2.x and 3.x  
**Project:** XR-4 Autonomous Rover

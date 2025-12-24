# XR4 Firmware Versioning Guide

## Overview
Simple semantic versioning system for both CTU and OBC firmware to track changes and ensure compatibility.

## Version Format
```
vMAJOR.MINOR.PATCH
```

### Version Components

**MAJOR** - Breaking changes or major feature additions
- Incompatible CTU ↔ OBC protocol changes
- Complete rewrites or architecture changes
- Major hardware changes requiring different configurations

**MINOR** - New features and improvements (backward compatible)
- New features added
- Performance improvements
- Optimizations that don't break compatibility

**PATCH** - Bug fixes and minor tweaks
- Bug fixes
- Small tweaks and adjustments
- Documentation updates

## Implementation

### Location
All version information is defined in:
```
lib/SN_Common/SN_Common.h
```

### Updating the Version
Edit these three defines in `SN_Common.h`:
```cpp
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 1
#define FIRMWARE_VERSION_PATCH 0
```

### Automatic Features
The system automatically generates:
- `FIRMWARE_VERSION_STRING` - "v1.1.0"
- `FIRMWARE_NAME` - "XR4-OBC" or "XR4-CTU" (board-specific)
- `FIRMWARE_FULL_NAME` - "XR4 On-Board Computer" or "XR4 Control & Telemetry Unit"
- `FIRMWARE_ID` - "XR4-OBC v1.1.0"
- `FIRMWARE_BUILD_DATE` - Compile date
- `FIRMWARE_BUILD_TIME` - Compile time

## Where Version is Displayed

### 1. Serial Monitor (Both CTU and OBC)
On startup, the firmware prints:
```
========================================
XR4 On-Board Computer
Version: v1.1.0
Build: Dec 24 2025 14:30:00
========================================
```

### 2. LCD Display (CTU Only)
The CTU LCD shows the version on the splash screen:
```
┌────────────────────┐
│    ROVER XR-4      │
│      v1.1.0        │
└────────────────────┘
```

## Version History

### v1.1.0 (Current)
- Ultra-low latency optimizations
  - Direct ISR motor control (<100µs response)
  - WiFi power save disabled (<1ms latency)
  - Zero-delay main loop (taskYIELD)
  - Instant headlights response
- Motor speed optimization (PWM 10kHz)
- GPIO12 strapping pin fix (Motor 4: GPIO15/13)
- Differential drive steering direction fix
- Added firmware versioning system

### v1.0.0 (Initial Release)
- Initial stable release
- Basic CTU and OBC functionality
- ESP-NOW communication
- Motor control with differential drive
- Joystick and switch inputs
- LCD display system
- Sensor integration
- GPS support

## Best Practices

### When to Update

**PATCH Version (+0.0.1)**
- Fixed a bug in motor control
- Adjusted joystick deadband
- Fixed LCD display issue
- Documentation updates

**MINOR Version (+0.1.0)**
- Added new LCD page
- Improved motor response time
- Added new sensor support
- Performance optimizations
- New features that don't break existing code

**MAJOR Version (+1.0.0)**
- Changed ESP-NOW packet structure (CTU ↔ OBC incompatible)
- Major hardware revision (different pin assignments)
- Complete rewrite of motor control system
- Breaking API changes

### Compatibility
- **MAJOR version must match** between CTU and OBC
- Different MINOR/PATCH versions can work together
- Always test CTU ↔ OBC communication after updates

### Release Process
1. Update version numbers in `SN_Common.h`
2. Update change log in `SN_Common.h` and this document
3. Compile and test both CTU and OBC
4. Verify version displays correctly on serial and LCD
5. Tag git commit with version: `git tag v1.1.0`
6. Upload to both boards

## Checking Firmware Version

### Via Serial Monitor
1. Connect board to computer
2. Open serial monitor at 115200 baud
3. Press RESET button on board
4. Version info appears in first few lines

### Via LCD (CTU Only)
1. Power on CTU
2. Version appears on splash screen for 1.5 seconds

### In Code
Access version information anywhere:
```cpp
#include <SN_Common.h>

// Print version
Serial.println(FIRMWARE_VERSION_STRING);  // "v1.1.0"
Serial.println(FIRMWARE_ID);              // "XR4-OBC v1.1.0"
Serial.println(FIRMWARE_BUILD_DATE);      // "Dec 24 2025"

// Check version programmatically
if (FIRMWARE_VERSION_MAJOR == 1 && FIRMWARE_VERSION_MINOR >= 1) {
    // Code that requires v1.1.0 or higher
}
```

## Future Enhancements

### Possible Additions
- Version exchange in ESP-NOW handshake (CTU tells OBC its version)
- Version mismatch warning on LCD
- Version stored in preferences for OTA update tracking
- Bootloader version separate from firmware version
- Git commit hash in build info

### Not Implemented (Keep It Simple)
- ❌ Build numbers (complexity)
- ❌ Pre-release tags (alpha/beta/rc)
- ❌ Metadata (+metadata)
- ❌ Date-based versioning (20251224)

---

**Remember:** Keep it simple! The goal is easy identification and compatibility checking, not complex version management.

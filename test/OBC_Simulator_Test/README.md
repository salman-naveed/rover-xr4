# OBC Simulator Test Sketch

## Overview
This Arduino sketch simulates an OBC (On-Board Computer) for testing the CTU (Control & Telemetry Unit) ESP-NOW communication without needing the actual rover hardware.

## Features
- ✅ Receives telecommands from CTU via ESP-NOW
- ✅ Displays detailed telecommand data on Serial Monitor
- ✅ Sends simulated telemetry (GPS, IMU, Housekeeping) back to CTU
- ✅ Cycles through all three telemetry types automatically
- ✅ Real-time status updates and statistics

## Hardware Requirements
- ESP32 development board (any variant: ESP32-WROOM, ESP32-S3, etc.)
- USB cable for programming and serial monitoring
- Computer with Arduino IDE or PlatformIO

## Setup Instructions

### Step 1: Get Your CTU's MAC Address
First, you need to find the MAC address of your CTU board:

1. Upload your main project to the CTU board
2. Open Serial Monitor at 115200 baud
3. Look for a line that says "CTU MAC Address: XX:XX:XX:XX:XX:XX"
4. Write down this MAC address

**Alternative method:** Add this to your CTU's setup():
```cpp
Serial.print("CTU MAC Address: ");
Serial.println(WiFi.macAddress());
```

### Step 2: Configure the Test Sketch
1. Open `OBC_Simulator_Test.ino`
2. Find this line (around line 28):
   ```cpp
   uint8_t ctuMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // CHANGE THIS!
   ```
3. Replace with your CTU's actual MAC address, for example:
   ```cpp
   uint8_t ctuMacAddress[] = {0x24, 0x6F, 0x28, 0xAB, 0xCD, 0xEF};
   ```

### Step 3: Upload to ESP32
1. Connect a **different** ESP32 board (not your CTU) via USB
2. Select the correct board and port in Arduino IDE
3. Upload the sketch
4. Open Serial Monitor at 115200 baud

### Step 4: Test with CTU
1. Power on your CTU board
2. The OBC simulator should start receiving telecommands
3. Watch the Serial Monitor for received data

## What You'll See

### Telecommand Display (from CTU)
```
========== TELECOMMAND RECEIVED ==========
Message Type: 0x11
Command: 0x0000
Joystick X: 2048 (Raw ADC)
Joystick Y: 2048 (Raw ADC)
Encoder Position: 5
CTU RSSI: -45 dBm

Control Flags:
  Emergency Stop: Inactive
  Armed: YES
  Button A: Released
  Button B: Released
  Button C: Released
  Button D: Released
  Headlights: ON
  Buzzer: OFF
==========================================
```

### Telemetry Status
```
✓ Telemetry sent successfully (GPS)
✓ Telemetry sent successfully (IMU)
✓ Telemetry sent successfully (Housekeeping)

--- Status Update ---
Telecommands received: 145
Telemetry packets sent: 87
Last telecommand: 234 ms ago
--------------------
```

## Simulated Telemetry Data

### GPS Telemetry (Type 0x10)
- Latitude: ~37.7749° (San Francisco area, with slow drift)
- Longitude: ~-122.4194°
- GPS Time: System uptime in seconds
- GPS Fix: Always true

### IMU Telemetry (Type 0x20)
- Gyroscope X/Y/Z: Sinusoidal motion (simulated rotation)
- Accelerometer X/Y/Z: Gravity + simulated motion
- Magnetometer X/Y/Z: Simulated magnetic field

### Housekeeping Telemetry (Type 0x30)
- Main Bus Voltage: 11.5-12.5V (simulated battery)
- Main Bus Current: 2.2-2.8A
- Temperature: 20-30°C
- OBC RSSI: -35 to -55 dBm (random)

## Telemetry Send Interval
Default: **500 ms** (2 Hz)

To change the interval, modify this line:
```cpp
#define TELEMETRY_SEND_INTERVAL_MS 500  // Change to desired value
```

Telemetry types are sent in rotation: GPS → IMU → Housekeeping → GPS → ...

## Troubleshooting

### "Failed to add CTU peer"
- **Cause:** Incorrect CTU MAC address
- **Fix:** Double-check the MAC address matches your CTU exactly

### No telecommands received
- **Cause 1:** CTU not transmitting
  - Check CTU is powered on and ESP-NOW initialized
  - Check CTU's Serial Monitor for send confirmations
  
- **Cause 2:** Wrong MAC address
  - Verify you entered the CTU MAC correctly
  
- **Cause 3:** WiFi channel mismatch
  - ESP-NOW should auto-match channels, but try rebooting both boards

### Telemetry not appearing on CTU LCD
- **Cause:** CTU not receiving telemetry
  - Check OBC simulator Serial Monitor shows "✓ Telemetry sent successfully"
  - Verify CTU has OBC peer registered (check CTU logs)
  - Try increasing telemetry interval to reduce packet loss

## Integration with Your CTU

The OBC simulator sends the exact same data structures as the real OBC, so:
- Your CTU's LCD should display the simulated telemetry values
- All ESP-NOW receive callbacks will trigger normally
- You can test your entire CTU workflow without the rover

## Testing Scenarios

### Test Emergency Stop
1. Press the E-STOP switch on your CTU
2. Watch the OBC simulator Serial Monitor
3. You should see `Emergency Stop: ACTIVE` in the telecommand

### Test Joystick
1. Move the joystick on your CTU
2. Watch the `Joystick X` and `Joystick Y` values change
3. Values should range from ~0 to ~4095

### Test Encoder Navigation
1. Rotate the encoder on your CTU
2. Watch `Encoder Position` increment/decrement
3. Press encoder button to see button flags

### Test ARM Switch
1. Toggle ARM switch on your CTU
2. Watch `Armed: YES/NO` change in the telecommand

## Code Structure

```
setup()
├── Initialize Serial
├── Initialize WiFi (Station mode)
├── Initialize ESP-NOW
├── Register send/receive callbacks
└── Add CTU as peer

loop()
├── Check telemetry send timer
│   ├── Update placeholder values
│   ├── Send GPS/IMU/HK (rotating)
│   └── Cycle to next telemetry type
└── Print status every 5 seconds
```

## MAC Address Format Conversion

If your CTU MAC is displayed as: `24:6F:28:AB:CD:EF`

Convert to array format:
```cpp
uint8_t ctuMacAddress[] = {0x24, 0x6F, 0x28, 0xAB, 0xCD, 0xEF};
```

## Notes
- The OBC simulator does NOT control motors (it's receive-only for telecommands)
- Telemetry values are mathematically generated, not from real sensors
- Perfect for testing CTU's LCD display, ESP-NOW reliability, and data handling
- Can run 24/7 for stress testing

## Expected Performance
- **Packet Rate:** ~6 packets/second (3 telemetry types × 2 Hz)
- **Latency:** <10ms typical for ESP-NOW
- **Reliability:** >95% packet delivery in good conditions

## Next Steps
Once you verify communication works with the simulator, you can:
1. Test the real OBC code with confidence
2. Debug CTU display issues without rover hardware
3. Develop new features on CTU side safely
4. Stress test ESP-NOW communication range

---

**Author:** GitHub Copilot  
**Date:** December 15, 2025  
**License:** Same as rover-xr4 project



## Analysis Complete: XR-4 OBC (On-Board Computer) Functionality

I've thoroughly analyzed the codebase with `SN_XR4_BOARD_TYPE=SN_XR4_OBC_ESP32` enabled. Here's a comprehensive overview of the OBC functionality:

---

## 🎯 **OBC Overview**

The **On-Board Computer (OBC)** is the ESP32-based autonomous controller mounted on the XR-4 rover. It receives control commands wirelessly from the CTU (Control & Telemetry Unit) and executes them while managing all rover subsystems.

---

## 🔧 **Core Subsystems (OBC-Specific)**

### 1. **Motor Control System** 🚗
- **Location**: SN_Motors
- **Hardware**: 4 DC motors controlled via MCPWM (Motor Control PWM)
  - **Left Front** (Motor 1): GPIO 32, 33
  - **Left Rear** (Motor 2): GPIO 25, 26
  - **Right Front** (Motor 3): GPIO 27, 14
  - **Right Rear** (Motor 4): GPIO 12, 13
- **Features**:
  - Differential drive control (left/right speed)
  - Acceleration ramping (smooth speed transitions)
  - Emergency stop capability
  - Forward/backward directional control
- **Function**: `SN_Motors_Drive(leftSpeed, rightSpeed)` - drives based on joystick input from CTU

### 2. **GPS Navigation System** 🛰️
- **Location**: SN_GPS
- **Hardware**: GPS module on GPIO 16 (RX), GPIO 17 (TX)
- **Features**:
  - Background GPS acquisition via Ticker (500ms interval)
  - Non-blocking initialization (rover starts even without GPS fix)
  - GPS health monitoring (10s timeout)
  - TinyGPSPlus library for NMEA parsing
- **Data Provided**:
  - Latitude/Longitude
  - GPS time
  - Fix status

### 3. **ESP-NOW Wireless Communication** 📡
- **Location**: SN_ESPNOW
- **Role**: **Receiver** (receives telecommands from CTU, sends telemetry back)
- **Communication Flow**:
  
  **Incoming (CTU → OBC):**
  - Telecommand packets with joystick, switches, commands
  - High-priority processing in ISR callback for E-STOP and ARM signals
  
  **Outgoing (OBC → CTU):**
  - GPS data (100ms interval)
  - IMU data (100ms interval) 
  - Housekeeping data (200ms interval) - voltage, current, temperature, RSSI

### 4. **Status Panel (Visual Feedback)** 💡
- **Location**: SN_StatusPanel
- **Hardware**: 
  - **2 NeoPixel LED strips** (8 LEDs each)
  - Strip 1: Status indicators
  - Strip 2: Headlights (controllable from CTU)
- **LED States**:
  - Solid Blue: Initializing
  - Moving Back/Forth: Waiting for ARM
  - Solid Green: Armed and operational
  - Blink Red: Emergency stop
  - Solid Red: Error state

---

## 🔄 **OBC Startup Sequence**

```
1. Power On → XR4_STATE_JUST_POWERED_ON
   ├── Initialize Serial Monitor (UART SLIP)
   ├── Log reset reason (watchdog, brownout, etc.)
   ├── Initialize Status Panel (LEDs)
   └── Initialize OBC peripherals:
       ├── SN_Motors_Init() - Configure 4-motor MCPWM system
       └── SN_GPS_Init() - Start GPS acquisition (non-blocking)

2. State → XR4_STATE_INITIALIZED
   └── LED: Solid Blue

3. State → XR4_STATE_COMMS_CONFIG
   └── SN_ESPNOW_Init()
       ├── Configure WiFi as Station
       ├── Initialize ESP-NOW protocol
       ├── Register send/receive callbacks
       └── Add CTU as peer (MAC: 0x24:0a:c4:c0:f1:ec)
       
   ✅ Success → XR4_STATE_WAITING_FOR_ARM (LED: Moving Back/Forth)
   ❌ Failure → XR4_STATE_ERROR (LED: Solid Red)

4. Main Loop (100-200 Hz)
   └── SN_OBC_MainHandler()
```

---

## 🎮 **OBC Main Handler Logic**

The **`SN_OBC_MainHandler()`** function is the heart of OBC operation:

```cpp
void SN_OBC_MainHandler() {
    // 1. Update context with received telecommands from CTU
    SN_Telecommand_updateContext(OBC_in_telecommand_data);
    
    // 2. Execute high-priority commands (E-STOP, ARM/DISARM, Headlights)
    SN_OBC_ExecuteCommands();
    
    // 3. Execute driving commands (joystick → motor speeds)
    SN_OBC_DrivingHandler();
    
    // 4. Update telemetry structs with sensor data
    SN_Telemetry_updateStruct(xr4_system_context);
    
    // 5. Send telemetry to CTU (GPS/IMU/HK at configured intervals)
    SN_ESPNOW_SendTelemetry();
}
```

---

## 🚨 **Safety & State Management**

### **State Machine** (6 states):

| State | LED | Behavior |
|-------|-----|----------|
| **WAITING_FOR_ARM** | Moving ↔ | Motors stopped, waiting for ARM signal from CTU |
| **ARMED** | Solid Green | Motors active, joystick control enabled |
| **EMERGENCY_STOP** | Blink Red | All motors stopped, ignores joystick input |
| **ERROR** | Solid Red | Communication failure, motors stopped |
| **COMMS_CONFIG** | Solid Blue | Initializing ESP-NOW |
| **INITIALIZED** | Solid Blue | Boot complete, transitioning to COMMS_CONFIG |

### **High-Priority Interrupt Handling**:
- **E-STOP**: Processed immediately in `OnTelecommandReceive()` ISR
  - Motors stopped within <100µs
  - State forced to `XR4_STATE_EMERGENCY_STOP`
- **DISARM**: Also processed in ISR
  - Motors stopped
  - State forced to `XR4_STATE_WAITING_FOR_ARM`

---

## 📊 **Data Flow**

### **Telecommand Processing (CTU → OBC)**:

```
CTU sends telecommand packet:
├── Joystick_X, Joystick_Y (raw ADC values)
├── Encoder position
├── Command field (future use)
└── Flags (8 bits):
    ├── Bit 7: Emergency_Stop ⚠️
    ├── Bit 6: Armed
    ├── Bit 5-2: Buttons A-D
    ├── Bit 1: Headlights_On 💡
    └── Bit 0: Buzzer

↓ OnTelecommandReceive() callback (ISR)
↓ SN_Telecommand_updateContext()
↓ xr4_system_context updated
↓ SN_OBC_ExecuteCommands() checks flags
↓ SN_OBC_DrivingHandler() maps joystick → motors
```

### **Telemetry Transmission (OBC → CTU)**:

```
Telemetry data collected from:
├── GPS: Lat/Lon, Time, Fix status
├── IMU: Gyro, Accel, Magnetometer (planned)
└── Housekeeping: Voltage, Current, Temp, RSSI

↓ SN_Telemetry_updateStruct() packs data
↓ SN_ESPNOW_SendTelemetry() rotates between 3 message types
↓ Sent at configured intervals (100ms/100ms/200ms)
```

---

## 🎛️ **Key Functions**

| Function | Purpose |
|----------|---------|
| `SN_Motors_Init()` | Configure 4-motor MCPWM system |
| `SN_Motors_Drive(left, right)` | Set differential drive speeds |
| `SN_Motors_Stop()` | Emergency stop all motors |
| `SN_GPS_Init()` | Start GPS acquisition (non-blocking) |
| `SN_ESPNOW_Init()` | Initialize wireless communication |
| `SN_OBC_MainHandler()` | Main control loop (telecommands → actions) |
| `SN_OBC_ExecuteCommands()` | Process E-STOP, ARM, Headlights |
| `SN_OBC_DrivingHandler()` | Map joystick → motor speeds |
| `OnTelecommandReceive()` | ISR callback for incoming CTU commands |
| `SN_StatusPanel__SetStatusLedState()` | Update LED status indicators |

---

## 🔑 **Key Differences: OBC vs CTU**

| Feature | OBC (Rover) | CTU (Remote) |
|---------|-------------|--------------|
| **Hardware** | 4 motors, GPS, 2 LED strips | Joystick, LCD, switches, encoder |
| **Role** | Command executor | Command generator |
| **ESP-NOW** | Receives telecommands, sends telemetry | Sends telecommands, receives telemetry |
| **Inputs** | Wireless commands from CTU | Physical joystick & switches |
| **Outputs** | Motor control, headlights | LCD display, status LEDs |
| **Safety** | Emergency stop via wireless | Emergency stop via hardware switch |

---

## 📋 **System Context Structure**

The `xr4_system_context_t` struct is shared between OBC and CTU:

```cpp
// OBC-specific fields (telemetry):
- GPS_lat, GPS_lon, GPS_time, GPS_fix
- Gyro_X/Y/Z, Acc_X/Y/Z, Mag_X/Y/Z (IMU)
- Main_Bus_V, Main_Bus_I, temp, OBC_RSSI

// CTU-specific fields (telecommands):
- Joystick_X, Joystick_Y, Encoder_Pos
- Emergency_Stop, Armed, Headlights_On
- Buttons A-D, Buzzer, Command
- CTU_RSSI

// Common:
- system_state (state machine)
```

---

## 🛡️ **Robustness Features**

1. **Non-blocking GPS**: Rover starts even without GPS fix
2. **ESP-NOW retry logic**: Cleans up and reinitializes on failure
3. **Hardware interrupt E-STOP**: <100µs response time
4. **State-based safety**: Motors only active in ARMED state
5. **Acceleration ramping**: Prevents motor current spikes
6. **Logging**: Comprehensive debug logging via UART SLIP

---

This OBC firmware is well-architected for autonomous rover operation with robust safety mechanisms, modular subsystems, and efficient wireless control. The state machine ensures predictable behavior, and the interrupt-driven E-STOP provides critical safety response times.

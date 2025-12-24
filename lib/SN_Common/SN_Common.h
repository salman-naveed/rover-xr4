#ifndef SN_COMMON_H
#define SN_COMMON_H

#include <Arduino.h>

// ============================================================================
// FIRMWARE VERSION INFORMATION
// ============================================================================
// Simple versioning scheme: MAJOR.MINOR.PATCH
// - MAJOR: Breaking changes, major feature additions
// - MINOR: New features, improvements (backward compatible)
// - PATCH: Bug fixes, minor tweaks
// 
// Change Log:
// v1.0.0 - Initial stable release
// v1.1.0 - Ultra-low latency optimizations (ISR motor control, WiFi tuning)
//        - Motor speed optimization (PWM frequency tuning)
//        - GPIO12 strapping pin fix
//        - Differential drive steering fix
// ============================================================================

#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 1
#define FIRMWARE_VERSION_PATCH 0

// Auto-generated version string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FIRMWARE_VERSION_STRING "v" TOSTRING(FIRMWARE_VERSION_MAJOR) "." TOSTRING(FIRMWARE_VERSION_MINOR) "." TOSTRING(FIRMWARE_VERSION_PATCH)

// Board-specific version identifiers
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  #define FIRMWARE_NAME "XR4-OBC"
  #define FIRMWARE_FULL_NAME "XR4 On-Board Computer"
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
  #define FIRMWARE_NAME "XR4-CTU"
  #define FIRMWARE_FULL_NAME "XR4 Control & Telemetry Unit"
#else
  #define FIRMWARE_NAME "XR4-UNKNOWN"
  #define FIRMWARE_FULL_NAME "XR4 Unknown Board"
#endif

// Complete firmware identification string
#define FIRMWARE_ID FIRMWARE_NAME " " FIRMWARE_VERSION_STRING

// Build date/time (automatically set at compile time)
#define FIRMWARE_BUILD_DATE __DATE__
#define FIRMWARE_BUILD_TIME __TIME__

// ============================================================================

// Device states for state machine
#define XR4_STATE_JUST_POWERED_ON 0
#define XR4_STATE_INITIALIZED 1
#define XR4_STATE_COMMS_CONFIG 2
#define XR4_STATE_WAITING_FOR_ARM 3
#define XR4_STATE_ARMED 4
#define XR4_STATE_ERROR 5
#define XR4_STATE_EMERGENCY_STOP 6
#define XR4_STATE_OTA_FW_UPDATE 7
#define XR4_STATE_REBOOT 8


typedef struct system_context {
    // Common fields (both OBC and CTU)
    uint8_t system_state;


    // OBC-specific fields (Telemetry, OBC -> CTU)
    double GPS_lat;
    double GPS_lon;
    double GPS_time;
    bool GPS_fix;

    float Gyro_X;
    float Gyro_Y;
    float Gyro_Z;
    
    float Acc_X;
    float Acc_Y;
    float Acc_Z;
    
    float Mag_X;
    float Mag_Y;
    float Mag_Z;
    
    float Main_Bus_V;
    float Main_Bus_I;
    
    float temp;
    float OBC_RSSI;

    // CTU-specific fields (Telecommand, CTU -> OBC)
    uint16_t Command;
    uint16_t Joystick_X;    // Joystick X-axis raw ADC value
    uint16_t Joystick_Y;    // Joystick Y-axis raw ADC value
    uint16_t Encoder_Pos;
    uint16_t CTU_RSSI;

    bool Emergency_Stop;
    bool Armed;
    bool Headlights_On;
    bool Buzzer;
    bool Button_A;
    bool Button_B;
    bool Button_C;
    bool Button_D;

} xr4_system_context_t;

#endif // SN_COMMON_H

#ifndef SN_COMMON_H
#define SN_COMMON_H

#include <Arduino.h>



// Device states for state machine
#define XR4_STATE_JUST_POWERED_ON 0
#define XR4_STATE_INITIALIZED 1
#define XR4_STATE_COMMS_CONFIG 2
#define XR4_STATE_WAITING_FOR_ARM 3
#define XR4_STATE_ARMED 4
#define XR4_STATE_ERROR 5
#define XR4_STATE_EMERGENCY_STOP 6
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


#include <Arduino.h>

typedef struct xr4_system_context {
    // OBC-specific fields (Telemetry, OBC -> CTU)
    float GPS_lat;
    float GPS_lon;
    float GPS_time;
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



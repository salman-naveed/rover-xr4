
#include <Arduino.h>

typedef struct xr4_system_context {
    float GPS_lat;
    float GPS_lon;
    float GPS_time;
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
    uint8_t OBC_RSSI;
    uint8_t Command;
    uint8_t Comm_Mode;
    uint8_t Joystick_X;     // Joystick raw ADC X axis value
    uint8_t Joystick_Y;     // Joystick raw ADC Y axis value
    uint8_t Emergency_Stop; // Emergency stop (1 = stop, 0 = go)
    uint8_t Armed;          // Armed (1 = armed, 0 = disarmed)
    uint8_t Button_A;       
    uint8_t Button_B;
    uint8_t Button_C;
    uint8_t Button_D;
    uint8_t Encoder_Pos;
    uint8_t CTU_RSSI;           // Received Signal Strength Indicator
    uint8_t Headlights_On;  // Headlights on (1 = on, 0 = off)
    uint8_t Buzzer;         // Buzzer (1 = on, 0 = off)

} xr4_system_context_t;



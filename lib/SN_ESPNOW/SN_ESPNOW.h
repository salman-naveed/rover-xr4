#pragma once
#include <Arduino.h>
// #include <esp_now.h>
#include <SN_XR_Board_Types.h>

// Create a struct_message to hold telemetry data (OBC --> CTU)
typedef struct OBC_telemetry_message {
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
    int RSSI;
} OBC_telemetry_message_t;

// Create a struct_message to hold telecommand data (CTU --> OBC)
typedef struct CTU_telecommand_message {
    uint8_t Command;
    uint8_t Comm_Mode;
    uint8_t Joystick_X;
    uint8_t Joystick_Y;
    uint8_t Emergency_Stop;
    uint8_t Armed;
    uint8_t Button_A;
    uint8_t Button_B;
    uint8_t Button_C;
    uint8_t Button_D;
    uint8_t Encoder_Pos;
    uint8_t RSSI;
    uint8_t Headlights_On;
    uint8_t Buzzer;


} CTU_telecommand_message_t;

void SN_ESPNOW_Init();

void SN_ESPNOW_register_send_cb();

void SN_ESPNOW_register_send_cb();

void SN_ESPNOW_register_recv_cb();

void SN_ESPNOW_add_peer();

void SN_ESPNOW_SendTelemetry();

void SN_ESPNOW_SendTelecommand();

void OnTelecommandReceive(const uint8_t * mac, const uint8_t *incoming_telecommand_data, int len);

void OnTelemetryReceive(const uint8_t * mac, const uint8_t *incoming_telemetry_data, int len);


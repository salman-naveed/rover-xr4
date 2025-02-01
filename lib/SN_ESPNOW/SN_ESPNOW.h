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
    float OBC_RSSI;
} OBC_telemetry_message_t;

// Create a struct_message to hold telecommand data (CTU --> OBC)
typedef struct CTU_telecommand_message {
    uint16_t Command;
    uint16_t Joystick_X;
    uint16_t Joystick_Y;
    uint16_t Encoder_Pos;
    uint16_t flags;         // Bytes structure (8-bit data): | Emergency_Stop | Armed | Button_A | Button_B | Button_C | Button_D | Headlights_On | Buzzer |
    uint16_t CTU_RSSI;
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


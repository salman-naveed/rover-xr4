#pragma once
#include <Arduino.h>
// #include <esp_now.h>
#include <SN_XR_Board_Types.h>
#include <SN_Common.h>

typedef enum {
    TM_GPS_DATA_MSG = 0x10,     // GPS data
    TM_IMU_DATA_MSG = 0x20,     // IMU data
    TM_HK_DATA_MSG = 0x30,      // Housekeeping data
} telemetry_message_type_id_t;

typedef enum {
    TC_C2_DATA_MSG = 0x11,      // Control & Commands data
} telecommand_message_type_id_t;

// Create telemetry data structures to hold telemetry data (OBC --> CTU) 
// instances of these structs are used by:
//  * CTU (when receiving TM from OBC): CTU_in
//  * OBC (when sending TM to CTU): OBC_out
typedef struct telemetry_GPS_data {
    uint8_t msg_type = TM_GPS_DATA_MSG;
    double GPS_lat;
    double GPS_lon;
    double GPS_time;
    
    bool GPS_fix;
} telemetry_GPS_data_t;

typedef struct telemetry_IMU_data {
    uint8_t msg_type = TM_IMU_DATA_MSG;
    float Gyro_X;
    float Gyro_Y;
    float Gyro_Z;
    float Acc_X;
    float Acc_Y;
    float Acc_Z;
    float Mag_X;
    float Mag_Y;
    float Mag_Z;
} telemetry_IMU_data_t;

typedef struct telemetry_HK_data {
    uint8_t msg_type = TM_HK_DATA_MSG;
    float Main_Bus_V;
    float Main_Bus_I;
    float temp;
    float OBC_RSSI;
} telemetry_HK_data_t;

// Create a struct_message to hold telecommand data (CTU --> OBC)
// instances used by:
//  * OBC (when received TC from CTU)
//  * CTU (when sending TC to OBC)
typedef struct telecommand_data {
    uint8_t msg_type = TC_C2_DATA_MSG;
    uint16_t Command;
    uint16_t Joystick_X;
    uint16_t Joystick_Y;
    uint16_t Encoder_Pos;
    uint16_t flags;         // Bytes structure (8-bit data): | Emergency_Stop | Armed | Button_A | Button_B | Button_C | Button_D | Headlights_On | Buzzer |
    uint16_t CTU_RSSI;
} telecommand_data_t;

// bool OBC_TC_received_data_ready;
// uint8_t OBC_TC_last_received_data_type;

// bool CTU_TM_received_data_ready;
// uint8_t CTU_TM_last_received_data_type;

void SN_ESPNOW_Init();

void SN_ESPNOW_register_send_cb();

void SN_ESPNOW_register_send_cb();

void SN_ESPNOW_register_recv_cb();

void SN_ESPNOW_add_peer();

void SN_ESPNOW_SendTelemetry();

void SN_ESPNOW_SendTelecommand(uint8_t telecommand_type);

void OnTelecommandReceive(const uint8_t * mac, const uint8_t *incoming_telecommand_data, int len);

void OnTelemetryReceive(const uint8_t * mac, const uint8_t *incoming_telemetry_data, int len);

void SN_Telemetry_updateContext(uint8_t CTU_TM_last_received_data_type);

void SN_Telemetry_updateStruct(xr4_system_context_t context);

void SN_Telecommand_updateContext(telecommand_data_t OBC_in_telecommand_data);

void SN_Telemetry_updateStruct(xr4_system_context_t context);

void SN_Telecommand_updateStruct(xr4_system_context_t context);



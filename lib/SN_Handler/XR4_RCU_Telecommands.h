#pragma once

#include <Arduino.h>

typedef enum: uint8_t {
    MSG_DATA_SW_UPGRADE_SETUP_ESP = 0xD1,

} RCU_Telecommand_Types;

// Communication Modes of the OBC, sent from the RCU in the telecommand message
typedef enum: uint8_t {
    SET_OBC_AS_ESPNOW_PEER_CONTROL_MODE = 0xD1,
    SET_OBC_AS_WIFI_AP_CONFIG_MODE = 0xD2,
    SET_OBC_AS_WIFI_STA_DIAGNOSTIC_MODE = 0xD3,
} OBC_Communication_Modes;


#define RCU_OP_MODE 0

#define OBC_OP_MODE SET_OBC_AS_ESPNOW_PEER_CONTROL_MODE

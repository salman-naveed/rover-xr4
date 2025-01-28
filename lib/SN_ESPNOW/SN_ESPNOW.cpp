#include <Arduino.h>
#include <esp_now.h>
#include <Wire.h>
#include <SN_ESPNOW.h>
#include <SN_Logger.h>
#include <SN_Common.h>
#include <SN_Handler.h>

extern xr4_system_context_t xr4_system_context;


esp_now_peer_info_t peerInfo;

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  // REPLACE WITH THE MAC OF THE CONTROL & TELEMETRY UNIT (CTU)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xc0, 0xe5, 0x78}; // MAC Address of the receiver (SN_XR4_CTU_ESP32 - Control/Telemetry Unit)

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
  // REPLACE WITH THE MAC OF THE ON-BOARD COMPUTER UNIT (OBC)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xbf, 0x9a, 0xe0}; // MAC Address of the receiver (SN_XR4_OBC_ESP32 - On-Board Computer Unit on the XR4 Rover)
#endif

// Create a struct_message to hold telemetry data (OBC --> RCU)
OBC_telemetry_message telemetry_message;

// Create a struct_message to hold telecommand data (RCU --> OBC)
CTU_telecommand_message telecommand_message;

// Variable to store if sending data was successful
String success;

void OnTelemetrySend(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnTelecommandSend(const uint8_t *mac_addr, esp_now_send_status_t status);


void SN_ESPNOW_Init(){
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    logMessage(true, "SN_ESPNOW_Init", "Error initializing ESP-NOW");
    return;
  }
    logMessage(true, "SN_ESPNOW_Init", "ESP-NOW initialized");

  delay(100);
}

void SN_ESPNOW_register_send_cb(){
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32  
        esp_now_register_send_cb(esp_now_send_cb_t(OnTelemetrySend));
        logMessage(true, "SN_ESPNOW_register_send_cb", "Send CB registered: OnTelemetrySend");
    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
        esp_now_register_send_cb(esp_now_send_cb_t(OnTelecommandSend));
        logMessage(true, "SN_ESPNOW_register_send_cb", "Send CB registered: OnTelecommandSend");
    #endif
}

void SN_ESPNOW_register_recv_cb(){
  // Register for a callback function that will be called when data is received
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
        esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelecommandReceive));
        logMessage(true, "SN_ESPNOW_register_recv_cb", "Recv CB registered: OnTelecommandReceive");
    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
        // esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelemetryReceive));
        logMessage(true, "SN_ESPNOW_register_recv_cb", "Recv CB registered: OnTelemetryReceive");
    #endif
}

void SN_ESPNOW_add_peer(){
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
    logMessage(true, "SN_ESPNOW_add_peer", "Peer added");
}

// ----------------- Data Send Functions -----------------
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void SN_ESPNOW_SendTelemetry(){
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &telemetry_message, sizeof(telemetry_message));

    if (result == ESP_OK) {
        logMessage(true, "SN_ESPNOW_SendTelemetry", "Sent with success");
    }
    else {
        logMessage(true, "SN_ESPNOW_SendTelemetry", "Error sending the data");
    }
}
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void SN_ESPNOW_SendTelecommand(){
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &telecommand_message, sizeof(telecommand_message));

    if (result == ESP_OK) {
        logMessage(true, "SN_ESPNOW_SendTelecommand", "Sent with success");
    }
    else {
        logMessage(true, "SN_ESPNOW_SendTelecommand", "Error sending the data");
    }
}
#endif
// --------------------------------------------------------

// ----------------- Update Telemetry / Telecommand Structs ---------------
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void SN_Telemetry_updateStruct(xr4_system_context_t context){
  telemetry_message.GPS_lat = context.GPS_lat;
  telemetry_message.GPS_lon = context.GPS_lon;
  telemetry_message.GPS_time = context.GPS_time;
  telemetry_message.Acc_X = context.Acc_X;
  telemetry_message.Acc_Y = context.Acc_Y;
  telemetry_message.Acc_Z = context.Acc_Z;
  telemetry_message.Gyro_X = context.Gyro_X;
  telemetry_message.Gyro_Y = context.Gyro_Y;
  telemetry_message.Gyro_Z = context.Gyro_Z;
  telemetry_message.Mag_X = context.Mag_X;
  telemetry_message.Mag_Y = context.Mag_Y;
  telemetry_message.Mag_Z = context.Mag_Z;
  telemetry_message.Main_Bus_V = context.Main_Bus_V;
  telemetry_message.Main_Bus_I = context.Main_Bus_I;
  telemetry_message.RSSI = context.OBC_RSSI;
  telemetry_message.temp = context.temp;
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void SN_Telecommand_updateStruct(xr4_system_context_t context){
  telecommand_message.Command = context.Command;
  telecommand_message.Comm_Mode = context.Comm_Mode;
  telecommand_message.Emergency_Stop = context.Emergency_Stop;
  telecommand_message.Armed = context.Armed;
  telecommand_message.Encoder_Pos = context.Encoder_Pos;
  telecommand_message.Joystick_X = context.Joystick_X;
  telecommand_message.Joystick_Y = context.Joystick_Y;
  telecommand_message.Button_A = context.Button_A;
  telecommand_message.Button_B = context.Button_B;
  telecommand_message.Button_C = context.Button_C;
  telecommand_message.Button_D = context.Button_D;
  telecommand_message.Headlights_On = context.Headlights_On;
  telecommand_message.RSSI = context.CTU_RSSI;

}

#endif


// ----------------- Data Send Callbacks -----------------
// Callback when data is sent
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void OnTelemetrySend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  logMessage(true, "OnTelemetrySend", "Telemetry Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "TM Delivery Success" : "TM Delivery Fail");  
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void OnTelecommandSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  logMessage(true, "OnTelecommandSend", "Telecommand Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "TC Delivery Success" : "TC Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}
#endif
// --------------------------------------------------------


// ---------------- Data Receive Callbacks ----------------
// Callback when data is received
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void OnTelecommandReceive(const uint8_t * mac, const uint8_t *incoming_telecommand_data, int len) {
    memcpy(&telecommand_message, incoming_telecommand_data, sizeof(telecommand_message));

    logMessage(true, "OnTelecommandReceive", "Telecommand received -----------------------");

    logMessage(true, "OnTelecommandReceive", "Bytes received: %d", len);

    logMessage(true, "OnTelecommandReceive", "Command: %d", telecommand_message.Command);
    logMessage(true, "OnTelecommandReceive", "Comm_Mode: %d", telecommand_message.Comm_Mode);
    logMessage(true, "OnTelecommandReceive", "Joystick_X: %d", telecommand_message.Joystick_X);
    logMessage(true, "OnTelecommandReceive", "Joystick_Y: %d", telecommand_message.Joystick_Y);
    logMessage(true, "OnTelecommandReceive", "Emergency_Stop: %d", telecommand_message.Emergency_Stop);
    logMessage(true, "OnTelecommandReceive", "Arm: %d", telecommand_message.Armed);
    logMessage(true, "OnTelecommandReceive", "Button_A: %d", telecommand_message.Button_A);
    logMessage(true, "OnTelecommandReceive", "Button_B: %d", telecommand_message.Button_B);
    logMessage(true, "OnTelecommandReceive", "Button_C: %d", telecommand_message.Button_C);
    logMessage(true, "OnTelecommandReceive", "Button_D: %d", telecommand_message.Button_D);
    logMessage(true, "OnTelecommandReceive", "Encoder_Pos: %d", telecommand_message.Encoder_Pos);

    logMessage(true, "OnTelecommandReceive", "------------------------------------------");

    // Update the OBC context with the received telecommand
    xr4_system_context.Command = telecommand_message.Command;
    xr4_system_context.Comm_Mode = telecommand_message.Comm_Mode;
    xr4_system_context.Joystick_X = telecommand_message.Joystick_X;
    xr4_system_context.Joystick_Y = telecommand_message.Joystick_Y;
    xr4_system_context.Emergency_Stop = telecommand_message.Emergency_Stop;
    xr4_system_context.Armed = telecommand_message.Armed;
    xr4_system_context.Button_A = telecommand_message.Button_A;
    xr4_system_context.Button_B = telecommand_message.Button_B;
    xr4_system_context.Button_C = telecommand_message.Button_C;
    xr4_system_context.Button_D = telecommand_message.Button_D;
    xr4_system_context.Encoder_Pos = telecommand_message.Encoder_Pos;
    xr4_system_context.RSSI = telecommand_message.RSSI;
    xr4_system_context.Headlights_On = telecommand_message.Headlights_On;
    xr4_system_context.Buzzer = telecommand_message.Buzzer;



}
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
// Callback when data is received
void OnTelemetryReceive(const uint8_t * mac, const uint8_t *incoming_telemetry_data, int len) {
    memcpy(&telemetry_message, incoming_telemetry_data, sizeof(telemetry_message));

    logMessage(true, "OnTelemetryReceive", "Bytes received: %d", len);

    // Update the CTU context with the received telemetry
    xr4_system_context.GPS_lat = telemetry_message.GPS_lat;
    xr4_system_context.GPS_lon = telemetry_message.GPS_lon;
    xr4_system_context.GPS_time = telemetry_message.GPS_time;
    xr4_system_context.Gyro_X = telemetry_message.Gyro_X;
    xr4_system_context.Gyro_Y = telemetry_message.Gyro_Y;
    xr4_system_context.Gyro_Z = telemetry_message.Gyro_Z;
    xr4_system_context.Acc_X = telemetry_message.Acc_X;
    xr4_system_context.Acc_Y = telemetry_message.Acc_Y;
    xr4_system_context.Acc_Z = telemetry_message.Acc_Z;
    xr4_system_context.Mag_X = telemetry_message.Mag_X;
    xr4_system_context.Mag_Y = telemetry_message.Mag_Y;
    xr4_system_context.Mag_Z = telemetry_message.Mag_Z;
    xr4_system_context.Main_Bus_V = telemetry_message.Main_Bus_V;
    xr4_system_context.Main_Bus_I = telemetry_message.Main_Bus_I;
    xr4_system_context.temp = telemetry_message.temp;
    xr4_system_context.RSSI = telemetry_message.RSSI;

}
#endif
// --------------------------------------------------------
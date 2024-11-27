#include <Arduino.h>
#include <esp_now.h>
#include <Wire.h>
#include <SN_ESPNOW.h>
#include <SN_Logger.h>

esp_now_peer_info_t peerInfo;

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  // REPLACE WITH THE MAC OF THE REMOTE CONTROLLER UNIT (RCU)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xc0, 0xe5, 0x78}; // MAC Address of the receiver (SN_XR4_RCU_ESP32 - Control/Telemetry Unit)

#elif SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
  // REPLACE WITH THE MAC OF THE ON-BOARD COMPUTER UNIT (OBC)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xbf, 0x9a, 0xe0}; // MAC Address of the receiver (SN_XR4_OBC_ESP32 - On-Board Computer Unit on the XR4 Rover)
#endif

// Create a struct_message to hold telemetry data (OBC --> RCU)
struct_telemetry_message telemetry_message;

// Create a struct_message to hold telecommand data (RCU --> OBC)
struct_telecommand_message telecommand_message;

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
    #elif SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
        esp_now_register_send_cb(esp_now_send_cb_t(OnTelecommandSend));
        logMessage(true, "SN_ESPNOW_register_send_cb", "Send CB registered: OnTelecommandSend");
    #endif
}

void SN_ESPNOW_register_recv_cb(){
  // Register for a callback function that will be called when data is received
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
        esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelecommandReceive));
        logMessage(true, "SN_ESPNOW_register_recv_cb", "Recv CB registered: OnTelecommandReceive");
    #elif SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
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
#elif SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
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
#elif SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
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
    logMessage(true, "OnTelecommandReceive", "Arm: %d", telecommand_message.Arm);
    logMessage(true, "OnTelecommandReceive", "Button_A: %d", telecommand_message.Button_A);
    logMessage(true, "OnTelecommandReceive", "Button_B: %d", telecommand_message.Button_B);
    logMessage(true, "OnTelecommandReceive", "Button_C: %d", telecommand_message.Button_C);
    logMessage(true, "OnTelecommandReceive", "Button_D: %d", telecommand_message.Button_D);
    logMessage(true, "OnTelecommandReceive", "Encoder_Pos: %d", telecommand_message.Encoder_Pos);

    logMessage(true, "OnTelecommandReceive", "------------------------------------------");


}
#elif SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
// Callback when data is received
void OnTelemetryReceive(const uint8_t * mac, const uint8_t *incoming_telemetry_data, int len) {
    memcpy(&telemetry_message, incoming_telemetry_data, sizeof(telemetry_message));

    logMessage(true, "OnTelemetryReceive", "Bytes received: %d", len);
}
#endif
// --------------------------------------------------------
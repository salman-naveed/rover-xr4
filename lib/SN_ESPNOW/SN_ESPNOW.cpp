#include <Arduino.h>
#include <esp_now.h>
#include <Wire.h>
#include <SN_ESPNOW.h>
#include <SN_Logger.h>
// #include <SN_Common.h>
#include <SN_Handler.h>
#include <SN_WiFi.h>

extern xr4_system_context_t xr4_system_context;

esp_now_peer_info_t peerInfo;



#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  // REPLACE WITH THE MAC OF THE CONTROL & TELEMETRY UNIT (CTU)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xc0, 0xe5, 0x78}; // MAC Address of the receiver (SN_XR4_CTU_ESP32 - Control/Telemetry Unit)

  bool OBC_TC_received_data_ready = false;
  uint8_t OBC_TC_last_received_data_type = 0;

  // OBC struct_message to hold outgoing telemetry data (OBC --> CTU)
  telemetry_GPS_data_t OBC_out_TM_GPS_data;
  telemetry_IMU_data_t OBC_out_TM_IMU_data;
  telemetry_HK_data_t OBC_out_TM_HK_data;

  // OBC struct_message to hold incoming telecommand data (CTU --> OBC)
  telecommand_data_t OBC_in_telecommand_data;

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
  // REPLACE WITH THE MAC OF THE ON-BOARD COMPUTER UNIT (OBC)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xbf, 0x9a, 0xe0}; // MAC Address of the receiver (SN_XR4_OBC_ESP32 - On-Board Computer Unit on the XR4 Rover)

  bool CTU_TM_received_data_ready = false;
  uint8_t CTU_TM_last_received_data_type = 0;

  // CTU struct_message to hold outgoing telecommand data (CTU --> OBC)
  telecommand_data_t CTU_out_telecommand_data;

  // CTU struct_message to hold incoming telemetry data (OBC --> CTU)
  telemetry_GPS_data_t CTU_in_TM_GPS_data;
  telemetry_IMU_data_t CTU_in_TM_IMU_data;
  telemetry_HK_data_t CTU_in_TM_HK_data;

#endif

// Variable to store if sending data was successful
String success;

void OnTelemetrySend(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnTelecommandSend(const uint8_t *mac_addr, esp_now_send_status_t status);

bool espnow_init_success = false;

// ----------------- Initialization Functions -----------------
void SN_ESPNOW_Init()
{
  SN_WiFi_StartAsWiFiClient();  // Set device as a Wi-Fi Station
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    logMessage(true, "SN_ESPNOW_Init", "Error initializing ESP-NOW");
    return;
  }
  else 
  {
    if(SN_ESPNOW_register_send_cb())        // Once ESPNOW is successfully initiated, we will register the Send CB function to get the status of trasnmitted packet
    {
      if(SN_ESPNOW_add_peer())              // Register and add peer
      {
        if(SN_ESPNOW_register_recv_cb())    // Register for a callback function that will be called when data is received
        {
          espnow_init_success = true;
          logMessage(true, "SN_ESPNOW_Init", "ESP-NOW initialized");
        }
      }
    }
    else if(!espnow_init_success)
    {
      logMessage(true, "SN_ESPNOW_Init", "ESP-NOW init failed");
    }
    
  }
  delay(100);
}

bool SN_ESPNOW_register_send_cb(){
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32  
        if(esp_now_register_send_cb(esp_now_send_cb_t(OnTelemetrySend)) != ESP_OK)
        {
          logMessage(true, "SN_ESPNOW_register_send_cb", "Send CB registered: OnTelemetrySend");
          return true;
        }
        else
          return false;
        
    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
        if(esp_now_register_send_cb(esp_now_send_cb_t(OnTelecommandSend)) != ESP_OK)
        {
          logMessage(true, "SN_ESPNOW_register_send_cb", "Send CB registered: OnTelecommandSend");
          return true;
        }
        else 
          return false;
    #endif
}

bool SN_ESPNOW_register_recv_cb(){
  // Register for a callback function that will be called when data is received
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
        if(esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelecommandReceive)) != ESP_OK)
        {
          logMessage(true, "SN_ESPNOW_register_recv_cb", "Recv CB registered: OnTelecommandReceive");
          return true;
        }
        else 
          return false;
        
    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
        if(esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelemetryReceive)) != ESP_OK)
        {
          logMessage(true, "SN_ESPNOW_register_recv_cb", "Recv CB registered: OnTelemetryReceive");
          return true;
        }
        else  
          return false;
        
    #endif
}

bool SN_ESPNOW_add_peer(){
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return false;
  }
    logMessage(true, "SN_ESPNOW_add_peer", "Peer added");
    return true;
}
// --------------------------------------------------------

// ----------------- Data Send Functions -----------------
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void SN_ESPNOW_SendTelemetry(uint8_t TM_out_msg_type){
    // Send message via ESP-NOW
    SN_Telemetry_updateStruct(xr4_system_context);

    esp_err_t result;

    if(TM_out_msg_type == TM_GPS_DATA_MSG){
      result = esp_now_send(broadcastAddress, (uint8_t *) &OBC_out_TM_GPS_data, sizeof(telemetry_GPS_data_t));
    }
    else if(TM_out_msg_type == TM_IMU_DATA_MSG){
      result = esp_now_send(broadcastAddress, (uint8_t *) &OBC_out_TM_IMU_data, sizeof(telemetry_IMU_data_t));
    }
    else if(TM_out_msg_type == TM_HK_DATA_MSG){
      result = esp_now_send(broadcastAddress, (uint8_t *) &OBC_out_TM_HK_data, sizeof(telemetry_HK_data_t));
    }

    if (result == ESP_OK) {
        logMessage(true, "SN_ESPNOW_SendTelemetry", "Sent with success");
    }
    else {
        logMessage(true, "SN_ESPNOW_SendTelemetry", "Error sending the data");
    }
}
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void SN_ESPNOW_SendTelecommand(uint8_t TC_out_msg_type){
    // Send message via ESP-NOW
    SN_Telecommand_updateStruct(xr4_system_context);

    esp_err_t result;

    if(TC_out_msg_type == TC_C2_DATA_MSG){
      result = esp_now_send(broadcastAddress, (uint8_t *) &CTU_out_telecommand_data, sizeof(telecommand_data_t));
    }

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
  if (status == 0){
    success = "Delivery Success";
  }
  else{
    success = "Delivery Fail";
  }
}
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void OnTelecommandSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  logMessage(true, "OnTelecommandSend", "Telecommand Send Status: %s", status == ESP_NOW_SEND_SUCCESS ? "TC Delivery Success" : "TC Delivery Fail");
  if (status == 0){
    success = "Delivery Success";
  }
  else{
    success = "Delivery Fail";
  }
}
#endif
// --------------------------------------------------------

// ----------------- Update Telemetry / Telecommand Structs before Sending ---------------
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void SN_Telemetry_updateStruct(xr4_system_context_t context){
  OBC_out_TM_GPS_data.GPS_lat = context.GPS_lat;
  OBC_out_TM_GPS_data.GPS_lon = context.GPS_lon;
  OBC_out_TM_GPS_data.GPS_time = context.GPS_time;
  OBC_out_TM_GPS_data.GPS_fix = context.GPS_fix;

  OBC_out_TM_IMU_data.Acc_X = context.Acc_X;
  OBC_out_TM_IMU_data.Acc_Y = context.Acc_Y;
  OBC_out_TM_IMU_data.Acc_Z = context.Acc_Z;
  OBC_out_TM_IMU_data.Gyro_X = context.Gyro_X;
  OBC_out_TM_IMU_data.Gyro_Y = context.Gyro_Y;
  OBC_out_TM_IMU_data.Gyro_Z = context.Gyro_Z;
  OBC_out_TM_IMU_data.Mag_X = context.Mag_X;
  OBC_out_TM_IMU_data.Mag_Y = context.Mag_Y;
  OBC_out_TM_IMU_data.Mag_Z = context.Mag_Z;

  OBC_out_TM_HK_data.Main_Bus_V = context.Main_Bus_V;
  OBC_out_TM_HK_data.Main_Bus_I = context.Main_Bus_I;
  OBC_out_TM_HK_data.OBC_RSSI = context.OBC_RSSI;
  OBC_out_TM_HK_data.temp = context.temp;
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void SN_Telecommand_updateStruct(xr4_system_context_t context){

  CTU_out_telecommand_data.Command = context.Command;
  CTU_out_telecommand_data.Joystick_X = context.Joystick_X;
  CTU_out_telecommand_data.Joystick_Y = context.Joystick_Y;
  CTU_out_telecommand_data.Encoder_Pos = context.Encoder_Pos;
  CTU_out_telecommand_data.flags = (context.Emergency_Stop << EMERGENCY_STOP_BIT) | (context.Armed << ARMED_BIT) | (context.Headlights_On << HEADLIGHTS_ON_BIT) | (context.Buzzer << BUZZER_BIT) | (context.Button_A << BUTTON_A_BIT) | (context.Button_B << BUTTON_B_BIT) | (context.Button_C << BUTTON_C_BIT) | (context.Button_D << BUTTON_D_BIT);
  CTU_out_telecommand_data.CTU_RSSI = context.CTU_RSSI;

}

#endif


// --------------------------------------------------------

// ----------------- Update OBC Context with Received Telecommand -----------------
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

void SN_Telecommand_updateContext(telecommand_data_t OBC_in_telecommand_data){
  if(OBC_TC_received_data_ready) {
    switch(OBC_TC_last_received_data_type) 
    {
      case TC_C2_DATA_MSG:
        xr4_system_context.Command = OBC_in_telecommand_data.Command;
        xr4_system_context.Joystick_X = OBC_in_telecommand_data.Joystick_X;
        xr4_system_context.Joystick_Y = OBC_in_telecommand_data.Joystick_Y;
        xr4_system_context.Encoder_Pos = OBC_in_telecommand_data.Encoder_Pos;
        xr4_system_context.CTU_RSSI = OBC_in_telecommand_data.CTU_RSSI;

        xr4_system_context.Emergency_Stop = get_flag(OBC_in_telecommand_data.flags, EMERGENCY_STOP_BIT);
        xr4_system_context.Armed = get_flag(OBC_in_telecommand_data.flags, ARMED_BIT);
        xr4_system_context.Headlights_On = get_flag(OBC_in_telecommand_data.flags, HEADLIGHTS_ON_BIT);
        xr4_system_context.Buzzer = get_flag(OBC_in_telecommand_data.flags, BUZZER_BIT);
        xr4_system_context.Button_A = get_flag(OBC_in_telecommand_data.flags, BUTTON_A_BIT);
        xr4_system_context.Button_B = get_flag(OBC_in_telecommand_data.flags, BUTTON_B_BIT);
        xr4_system_context.Button_C = get_flag(OBC_in_telecommand_data.flags, BUTTON_C_BIT);
        xr4_system_context.Button_D = get_flag(OBC_in_telecommand_data.flags, BUTTON_D_BIT);
        break;
    }
    OBC_TC_received_data_ready = false;
  }

}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

void SN_Telemetry_updateContext(uint8_t CTU_TM_last_received_data_type){
  if(CTU_TM_received_data_ready){
    switch(CTU_TM_last_received_data_type)
    {
      case TM_GPS_DATA_MSG:
        xr4_system_context.GPS_lat = CTU_in_TM_GPS_data.GPS_lat;
        xr4_system_context.GPS_lon = CTU_in_TM_GPS_data.GPS_lon;
        xr4_system_context.GPS_time = CTU_in_TM_GPS_data.GPS_time;
        break;

      case TM_IMU_DATA_MSG:
        xr4_system_context.Gyro_X = CTU_in_TM_IMU_data.Gyro_X;
        xr4_system_context.Gyro_Y = CTU_in_TM_IMU_data.Gyro_Y;
        xr4_system_context.Gyro_Z = CTU_in_TM_IMU_data.Gyro_Z;
        xr4_system_context.Acc_X = CTU_in_TM_IMU_data.Acc_X;
        xr4_system_context.Acc_Y = CTU_in_TM_IMU_data.Acc_Y;
        xr4_system_context.Acc_Z = CTU_in_TM_IMU_data.Acc_Z;
        xr4_system_context.Mag_X = CTU_in_TM_IMU_data.Mag_X;
        xr4_system_context.Mag_Y = CTU_in_TM_IMU_data.Mag_Y;
        xr4_system_context.Mag_Z = CTU_in_TM_IMU_data.Mag_Z;
        break;

      case TM_HK_DATA_MSG:
        xr4_system_context.Main_Bus_V = CTU_in_TM_HK_data.Main_Bus_V;
        xr4_system_context.Main_Bus_I = CTU_in_TM_HK_data.Main_Bus_I;
        xr4_system_context.temp = CTU_in_TM_HK_data.temp;
        xr4_system_context.OBC_RSSI = CTU_in_TM_HK_data.OBC_RSSI;
        break;
    }
    CTU_TM_received_data_ready = false;
  }

}

#endif
// --------------------------------------------------------

// ---------------- Data Receive Callbacks ----------------
// Callback when data is received
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void OnTelecommandReceive(const uint8_t * mac, const uint8_t *incoming_telecommand_data, int len) {

  wifi_pkt_rx_ctrl_t *rx_ctrl = (wifi_pkt_rx_ctrl_t *)incoming_telecommand_data;
  int data_len = len - rx_ctrl->sig_len;
  xr4_system_context.CTU_RSSI = rx_ctrl->rssi;
  
  uint8_t OBC_in_TM_msg_type;

  memcpy(&OBC_in_TM_msg_type, incoming_telecommand_data, sizeof(uint8_t));

  if(OBC_in_TM_msg_type == TC_C2_DATA_MSG){
    memcpy(&OBC_in_telecommand_data, incoming_telecommand_data, sizeof(telecommand_data_t));
    OBC_TC_last_received_data_type = TC_C2_DATA_MSG;
  }
  else {
    logMessage(true, "OnTelecommandReceive", "Unknown Telecommand Message Type Received");
  }

  logMessage(true, "OnTelecommandReceive", "Telecommand received -----------------------");
  logMessage(true, "OnTelecommandReceive", "Bytes received: %d", len);
  logMessage(true, "OnTelecommandReceive", "Command: %d", OBC_in_telecommand_data.Command);
  logMessage(true, "OnTelecommandReceive", "Joystick_X: %d", OBC_in_telecommand_data.Joystick_X);
  logMessage(true, "OnTelecommandReceive", "Joystick_Y: %d", OBC_in_telecommand_data.Joystick_Y);
  logMessage(true, "OnTelecommandReceive", "Encoder_Pos: %d", OBC_in_telecommand_data.Encoder_Pos);
  logMessage(true, "OnTelecommandReceive", "Flags: %d", OBC_in_telecommand_data.flags);
  logMessage(true, "OnTelecommandReceive", "------------------------------------------");

  OBC_TC_received_data_ready = true;
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
// Callback when a Telemetry data message is received by CTU
void OnTelemetryReceive(const uint8_t * mac, const uint8_t *incoming_telemetry_data, int len) {

  wifi_pkt_rx_ctrl_t *rx_ctrl = (wifi_pkt_rx_ctrl_t *)incoming_telemetry_data;
  int data_len = len - rx_ctrl->sig_len;
  xr4_system_context.CTU_RSSI = rx_ctrl->rssi;

  uint8_t CTU_in_TM_msg_type;

  memcpy(&CTU_in_TM_msg_type, incoming_telemetry_data, sizeof(uint8_t));

  if(CTU_in_TM_msg_type == TM_GPS_DATA_MSG){
    memcpy(&CTU_in_TM_GPS_data, incoming_telemetry_data, sizeof(telemetry_GPS_data_t));
    CTU_TM_last_received_data_type = TM_GPS_DATA_MSG;
    CTU_TM_received_data_ready = true;
  }
  else if(CTU_in_TM_msg_type == TM_IMU_DATA_MSG){
    memcpy(&CTU_in_TM_IMU_data, incoming_telemetry_data, sizeof(telemetry_IMU_data_t));
    CTU_TM_last_received_data_type = TM_IMU_DATA_MSG;
    CTU_TM_received_data_ready = true;
  }
  else if(CTU_in_TM_msg_type == TM_HK_DATA_MSG){
    memcpy(&CTU_in_TM_HK_data, incoming_telemetry_data, sizeof(telemetry_HK_data_t));
    CTU_TM_last_received_data_type = TM_HK_DATA_MSG;
    CTU_TM_received_data_ready = true;
  }
  else {
    logMessage(true, "OnTelemetryReceive", "Unknown Telemetry Message Type Received");
    return;
  }

  logMessage(true, "OnTelemetryReceive", "Telemetry received -----------------------");
  logMessage(true, "OnTelemetryReceive", "Bytes received: %d", len);
  logMessage(true, "OnTelemetryReceive", "------------------------------------------");
  
}
#endif
// --------------------------------------------------------
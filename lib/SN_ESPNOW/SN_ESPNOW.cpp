#include <Arduino.h>
#include <esp_now.h>
#include <Wire.h>
#include <SN_ESPNOW.h>
#include <SN_Logger.h>
// #include <SN_Common.h>
#include <SN_StatusPanel.h> // Include for LED_State and SN_StatusPanel__SetStatusLedState
#include <SN_Handler.h>
#include <SN_WiFi.h>
#include <SN_XR_Board_Types.h>
#include <SN_Motors.h>
#include <SN_Handler.h>

extern xr4_system_context_t xr4_system_context;

esp_now_peer_info_t peerInfo;



#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  // REPLACE WITH THE MAC OF THE CONTROL & TELEMETRY UNIT (CTU)
  uint8_t broadcastAddress[] = {0x24, 0x0a, 0xc4, 0xc0, 0xf1, 0xec}; //{0x24, 0x0a, 0xc4, 0xc0, 0xe5, 0x78}; // MAC Address of the receiver (SN_XR4_CTU_ESP32 - Control/Telemetry Unit)

  bool OBC_TC_received_data_ready = false;
  uint8_t OBC_TC_last_received_data_type = 0;

  // OBC struct_message to hold outgoing telemetry data (OBC --> CTU)
  telemetry_GPS_data_t OBC_out_TM_GPS_data;
  telemetry_IMU_data_t OBC_out_TM_IMU_data;
  telemetry_HK_data_t OBC_out_TM_HK_data;

  // OBC struct_message to hold incoming telecommand data (CTU --> OBC)
  telecommand_data_t OBC_in_telecommand_data;

  #define NUM_TM_MSG_TYPES 3

  static const telemetry_message_type_id_t telemetry_msg_types[NUM_TM_MSG_TYPES] = {
    TM_GPS_DATA_MSG,
    TM_IMU_DATA_MSG,
    TM_HK_DATA_MSG
  };

  // Configuration: Intervals in milliseconds for each message type
  static const uint32_t telemetry_intervals_ms[NUM_TM_MSG_TYPES] = {
    100, // TM_GPS_DATA_MSG interval
    100,  // TM_IMU_DATA_MSG interval
    200  // TM_HK_DATA_MSG interval
  };

  // Last sent timestamps for each message type (in microseconds)
  static uint64_t last_sent_time_us[NUM_TM_MSG_TYPES] = {0};

  // Static index for rotating through message types
  static uint8_t current_tm_index = 0;


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

  // Connection timeout tracking
  static unsigned long last_telemetry_received_time = 0;
  #define ESPNOW_TIMEOUT_MS 1000  // 1 second timeout

  // Diagnostic counters
  static uint32_t telemetry_packets_received = 0;
  static uint32_t telecommand_packets_sent = 0;
  static uint32_t telecommand_send_failures = 0;

#endif

// Variable to store if sending data was successful
String success;

void OnTelemetrySend(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnTelecommandSend(const uint8_t *mac_addr, esp_now_send_status_t status);


// ----------------- Initialization Functions -----------------
bool SN_ESPNOW_Init()
{
    logMessage(true, "SN_ESPNOW_Init", "Starting ESP-NOW initialization...");

    // Step 1: Configure device as Wi-Fi Station
    SN_WiFi_StartAsWiFiClient();

    // Step 2: Initialize ESP-NOW (deinit first if already initialized)
    esp_err_t init_result = esp_now_init();
    if (init_result == ESP_ERR_ESPNOW_BASE) {
        // ESP-NOW already initialized, deinit and try again
        logMessage(true, "SN_ESPNOW_Init", "ESP-NOW already initialized, reinitializing...");
        esp_now_deinit();
        delay(100);
        init_result = esp_now_init();
    }
    
    if (init_result != ESP_OK) {
        logMessage(true, "SN_ESPNOW_Init", "Error initializing ESP-NOW: %d", init_result);
        SN_ESPNOW_DeinitOnError();
        return false;
    }

    // Step 3: Register send callback
    if (!SN_ESPNOW_register_send_cb()) {
        logMessage(true, "SN_ESPNOW_Init", "Failed to register send callback");
        SN_ESPNOW_DeinitOnError();
        return false;
    }

    // Step 4: Add peer
    if (!SN_ESPNOW_add_peer()) {
        logMessage(true, "SN_ESPNOW_Init", "Failed to add ESP-NOW peer");
        SN_ESPNOW_DeinitOnError();
        return false;
    }

    // Step 5: Register receive callback
    if (!SN_ESPNOW_register_recv_cb()) {
        logMessage(true, "SN_ESPNOW_Init", "Failed to register receive callback");
        SN_ESPNOW_DeinitOnError();
        return false;
    }

    // ✅ Success
    logMessage(true, "SN_ESPNOW_Init", "ESP-NOW successfully initialized");
    delay(100);
    return true;
}

// --- Helper for consistent cleanup ---
void SN_ESPNOW_DeinitOnError()
{
  esp_now_deinit();
  logMessage(true, "SN_ESPNOW_DeinitOnError", "ESP-NOW deinitialized due to error");
  delay(100);
}

bool SN_ESPNOW_register_send_cb(){
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32  
        if(esp_now_register_send_cb(esp_now_send_cb_t(OnTelemetrySend)) == ESP_OK)
        {
          logMessage(true, "SN_ESPNOW_register_send_cb", "Send CB registered: OnTelemetrySend");
          return true;
        }
        else
          return false;
        
    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
        if(esp_now_register_send_cb(esp_now_send_cb_t(OnTelecommandSend)) == ESP_OK)
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
        if(esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelecommandReceive)) == ESP_OK)
        {
          logMessage(true, "SN_ESPNOW_register_recv_cb", "Recv CB registered: OnTelecommandReceive");
          return true;
        }
        else 
          return false;
        
    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
        if(esp_now_register_recv_cb(esp_now_recv_cb_t(OnTelemetryReceive)) == ESP_OK)
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
  
  // Check if peer already exists and remove it first
  if (esp_now_is_peer_exist(broadcastAddress)) {
    logMessage(true, "SN_ESPNOW_add_peer", "Peer already exists, removing first");
    esp_now_del_peer(broadcastAddress);
  }
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    logMessage(true, "SN_ESPNOW_add_peer", "Failed to add peer");
    return false;
  }
    logMessage(true, "SN_ESPNOW_add_peer", "Peer added successfully");
    return true;
}
// --------------------------------------------------------

// ----------------- Data Send Functions -----------------
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

void SN_ESPNOW_SendTelemetry(void) {
  // Use current_tm_index (0, 1, 2) to access the arrays properly
  telemetry_message_type_id_t msg_type = telemetry_msg_types[current_tm_index];

  // Calculate elapsed time since last send using the INDEX, not the enum value
  uint64_t now_us = esp_timer_get_time(); // Get current time in microseconds
  uint32_t elapsed_ms = (now_us - last_sent_time_us[current_tm_index]) / 1000;

  if (elapsed_ms >= telemetry_intervals_ms[current_tm_index]) {
      SN_Telemetry_updateStruct(xr4_system_context);
      esp_err_t result;

      switch (msg_type) {
          case TM_GPS_DATA_MSG:
              result = esp_now_send(broadcastAddress, (uint8_t *)&OBC_out_TM_GPS_data, sizeof(telemetry_GPS_data_t));
              break;
          case TM_IMU_DATA_MSG:
              result = esp_now_send(broadcastAddress, (uint8_t *)&OBC_out_TM_IMU_data, sizeof(telemetry_IMU_data_t));
              break;
          case TM_HK_DATA_MSG:
              result = esp_now_send(broadcastAddress, (uint8_t *)&OBC_out_TM_HK_data, sizeof(telemetry_HK_data_t));
              break;
          default:
              result = ESP_FAIL;
              break;
      }

      if (result == ESP_OK) {
          last_sent_time_us[current_tm_index] = now_us; // Update last sent time using INDEX
      }
  }

  // Cycle to next message type (0, 1, 2)
  current_tm_index = (current_tm_index + 1) % NUM_TM_MSG_TYPES;
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void SN_ESPNOW_SendTelecommand(uint8_t TC_out_msg_type){
    // Send message via ESP-NOW
    SN_Telecommand_updateStruct(xr4_system_context);

    esp_err_t result;

    if(TC_out_msg_type == TC_C2_DATA_MSG){
      result = esp_now_send(broadcastAddress, (uint8_t *) &CTU_out_telecommand_data, sizeof(telecommand_data_t));
      telecommand_packets_sent++;  // Diagnostic counter
    }
}
#endif
// --------------------------------------------------------

// ----------------- Data Send Callbacks -----------------
// Callback when data is sent
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
void OnTelemetrySend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Removed logging for performance
  if (status == 0){
    success = "Delivery Success";
  }
  else{
    success = "Delivery Fail";
  }
}
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
void OnTelecommandSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Removed logging for performance
  if (status == 0){
    success = "Delivery Success";
  }
  else{
    success = "Delivery Fail";
    telecommand_send_failures++;  // Diagnostic counter
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

  // Update orientation data (heading, pitch, roll)
  OBC_out_TM_IMU_data.Heading_Degrees = context.Heading_Degrees;
  strncpy(OBC_out_TM_IMU_data.Heading_Cardinal, context.Heading_Cardinal, 2);
  OBC_out_TM_IMU_data.Heading_Cardinal[2] = '\0';
  OBC_out_TM_IMU_data.Pitch_Degrees = context.Pitch_Degrees;
  OBC_out_TM_IMU_data.Roll_Degrees = context.Roll_Degrees;

  OBC_out_TM_HK_data.Main_Bus_V = context.Main_Bus_V;
  OBC_out_TM_HK_data.Main_Bus_I = context.Main_Bus_I;
  OBC_out_TM_HK_data.Bus_5V = context.Bus_5V;
  OBC_out_TM_HK_data.Bus_3V3 = context.Bus_3V3;
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

// PERFORMANCE CRITICAL: This function is called every main loop iteration
// Optimized for minimum latency by eliminating unnecessary operations
void SN_Telecommand_updateContext(const telecommand_data_t& OBC_in_telecommand_data){
  if(OBC_TC_received_data_ready) {
    // Direct assignment - no switch needed for single message type
    xr4_system_context.Command = OBC_in_telecommand_data.Command;
    xr4_system_context.Joystick_X = OBC_in_telecommand_data.Joystick_X;
    xr4_system_context.Joystick_Y = OBC_in_telecommand_data.Joystick_Y;
    xr4_system_context.Encoder_Pos = OBC_in_telecommand_data.Encoder_Pos;
    xr4_system_context.CTU_RSSI = OBC_in_telecommand_data.CTU_RSSI;

    // Inline flag extraction - faster than function calls
    uint16_t flags = OBC_in_telecommand_data.flags;
    xr4_system_context.Emergency_Stop = (flags >> EMERGENCY_STOP_BIT) & 1;
    xr4_system_context.Armed = (flags >> ARMED_BIT) & 1;
    xr4_system_context.Headlights_On = (flags >> HEADLIGHTS_ON_BIT) & 1;
    xr4_system_context.Buzzer = (flags >> BUZZER_BIT) & 1;
    xr4_system_context.Button_A = (flags >> BUTTON_A_BIT) & 1;
    xr4_system_context.Button_B = (flags >> BUTTON_B_BIT) & 1;
    xr4_system_context.Button_C = (flags >> BUTTON_C_BIT) & 1;
    xr4_system_context.Button_D = (flags >> BUTTON_D_BIT) & 1;
    
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
        // Update orientation data (heading, pitch, roll)
        xr4_system_context.Heading_Degrees = CTU_in_TM_IMU_data.Heading_Degrees;
        strncpy(xr4_system_context.Heading_Cardinal, CTU_in_TM_IMU_data.Heading_Cardinal, 2);
        xr4_system_context.Heading_Cardinal[2] = '\0';
        xr4_system_context.Pitch_Degrees = CTU_in_TM_IMU_data.Pitch_Degrees;
        xr4_system_context.Roll_Degrees = CTU_in_TM_IMU_data.Roll_Degrees;
        break;

      case TM_HK_DATA_MSG:
        xr4_system_context.Main_Bus_V = CTU_in_TM_HK_data.Main_Bus_V;
        xr4_system_context.Main_Bus_I = CTU_in_TM_HK_data.Main_Bus_I;
        xr4_system_context.Bus_5V = CTU_in_TM_HK_data.Bus_5V;
        xr4_system_context.Bus_3V3 = CTU_in_TM_HK_data.Bus_3V3;
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

  // OPTIMIZED: Direct RSSI capture without intermediate variable
  xr4_system_context.CTU_RSSI = ((wifi_pkt_rx_ctrl_t *)incoming_telecommand_data)->rssi;
  xr4_system_context.OBC_RSSI = xr4_system_context.CTU_RSSI;  // Same value for bidirectional link
  
  // OPTIMIZED: Check message type inline without intermediate variable
  if(*(uint8_t*)incoming_telecommand_data == TC_C2_DATA_MSG){
    // OPTIMIZED: Direct memcpy without type checking - we know it's TC_C2_DATA_MSG
    memcpy(&OBC_in_telecommand_data, incoming_telecommand_data, sizeof(telecommand_data_t));
    OBC_TC_last_received_data_type = TC_C2_DATA_MSG;
    OBC_TC_received_data_ready = true;

    SN_OBC_DrivingHandler();
    
    // // LATENCY CRITICAL: Process motor commands IMMEDIATELY in ISR for fastest response
    // // This bypasses the main loop entirely for time-critical motor control
    // if(!xr4_system_context.Emergency_Stop && xr4_system_context.Armed) {
    //   // Inline joystick mapping for zero-latency motor response
    //   int16_t raw_x = (int16_t)OBC_in_telecommand_data.Joystick_X - 2117;
    //   int16_t throttle = (abs(raw_x) < 50) ? 0 : (int16_t)(((int32_t)OBC_in_telecommand_data.Joystick_X * 200) / 4095) - 100;
      
    //   int16_t raw_y = (int16_t)OBC_in_telecommand_data.Joystick_Y - 2000;
    //   int16_t steering = (abs(raw_y) < 50) ? 0 : (int16_t)(((int32_t)OBC_in_telecommand_data.Joystick_Y * 200) / 4095) - 100;
      
    //   // Differential drive calculation
    //   // When joystick LEFT: steering is negative → left motors slower, right motors faster → turn LEFT
    //   // When joystick RIGHT: steering is positive → left motors faster, right motors slower → turn RIGHT
    //   int16_t leftSpeed = throttle + steering;
    //   int16_t rightSpeed = throttle - steering;
      
    //   // Clamp
    //   if (leftSpeed > 100) leftSpeed = 100;
    //   else if (leftSpeed < -100) leftSpeed = -100;
    //   if (rightSpeed > 100) rightSpeed = 100;
    //   else if (rightSpeed < -100) rightSpeed = -100;
      
    //   // Drive motors IMMEDIATELY - no waiting for main loop!
    //   SN_Motors_Drive(leftSpeed, rightSpeed);
    // } else {
    //   // Safety: Stop motors if ESTOP or disarmed
    //   SN_Motors_Drive(0, 0);
    // }
    
    // LATENCY CRITICAL: Handle headlights IMMEDIATELY as well
    // Extract headlights flag directly from telecommand flags
    bool headlights_on = (OBC_in_telecommand_data.flags >> 1) & 0x01;  // Bit 1 is headlights
    if(headlights_on) {
      SN_StatusPanel__ControlHeadlights(true);
    } else {
      SN_StatusPanel__ControlHeadlights(false);
    }
  }

  // Main loop will still update context for other peripherals (sensors, telemetry, etc.)
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
// Callback when a Telemetry data message is received by CTU
void OnTelemetryReceive(const uint8_t * mac, const uint8_t *incoming_telemetry_data, int len) {

  // Track last received time for connection timeout detection
  last_telemetry_received_time = millis();
  telemetry_packets_received++;  // Diagnostic counter

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
  
}
#endif
// --------------------------------------------------------

// ----------------- Connection Status Check (CTU Only) -----------------
#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
/**
 * Check if ESP-NOW connection is active
 * Returns true if telemetry packets received within timeout period
 */
bool SN_ESPNOW_IsConnected() {
  // Check if we've received telemetry recently
  return (millis() - last_telemetry_received_time) < ESPNOW_TIMEOUT_MS;
}

/**
 * Get diagnostic counter values
 */
uint32_t SN_ESPNOW_GetTelemetryPacketsReceived() {
  return telemetry_packets_received;
}

uint32_t SN_ESPNOW_GetTelecommandPacketsSent() {
  return telecommand_packets_sent;
}

uint32_t SN_ESPNOW_GetTelecommandSendFailures() {
  return telecommand_send_failures;
}
#endif
// --------------------------------------------------------
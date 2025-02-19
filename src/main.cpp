#include <Arduino.h>
#include <SN_XR_Board_Types.h>
#include <SN_ESPNOW.h>
#include <SN_WiFi.h>
#include <SN_Logger.h>
#include <SN_UART_SLIP.h>
#include <SN_Handler.h>
 
void setup() {
  // Init Serial Monitor
  SN_UART_SLIP_Init();

  // Set device as a Wi-Fi Station
  SN_WiFi_StartAsWiFiClient();

  SN_ESPNOW_Init();

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  SN_ESPNOW_register_send_cb();
  
  // Register and add peer
  SN_ESPNOW_add_peer();

  // Register for a callback function that will be called when data is received
  SN_ESPNOW_register_recv_cb();
}
 
void loop() {
  #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
    // OBC Handler
    SN_OBC_MainHandler();

  #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
    // CTU Handler
    SN_CTU_MainHandler();
  #endif
}


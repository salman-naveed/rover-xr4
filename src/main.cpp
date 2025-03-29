#include <Arduino.h>
#include <SN_XR_Board_Types.h>
#include <SN_ESPNOW.h>
#include <SN_Logger.h>
#include <SN_UART_SLIP.h>
#include <SN_Handler.h>
#include <SN_LCD.h>

extern bool espnow_init_success;
 
void setup() {
  SN_UART_SLIP_Init();   // Init Serial Monitor

  #if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
    SN_LCD_Init();
  #endif

  SN_ESPNOW_Init();


}
 
void loop() {
  if(espnow_init_success)
  {
    #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
      // OBC Handler
      SN_OBC_MainHandler();

    #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
      // CTU Handler
      SN_CTU_MainHandler();
    #endif
  }

}


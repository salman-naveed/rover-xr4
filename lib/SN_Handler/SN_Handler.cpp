#include <Arduino.h>
#include <SN_Handler.h>
#include <SN_ESPNOW.h>
#include <SN_Common.h>

extern xr4_system_context_t xr4_system_context;


#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
#include <XR4_RCU_Telecommands.h>
#define JOYSTICK_X_AXIS 0
#define JOYSTICK_Y_AXIS 1

extern CTU_telecommand_message_t telecommand_message;

uint8_t EMERGENCY_STOP_ACTIVE = 0;
uint8_t joystick_test_x = 0;
uint8_t joystick_test_y = 0;

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
extern OBC_telemetry_message_t telemetry_message;

#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

void SN_CTU_Handler(){
  // RCU Handler
  SN_CTU_compile_Telecommand_Message();

  SN_ESPNOW_SendTelecommand();

  delay(1000);
}

CTU_telecommand_message_t SN_CTU_compile_Telecommand_Message() {
  telecommand_message.Command = xr4_system_context.Command;
  telecommand_message.Comm_Mode = SN_CTU_get_OBC_Communication_Mode();
  telecommand_message.Joystick_X = xr4_system_context.Joystick_X;
  telecommand_message.Joystick_Y = xr4_system_context.Joystick_Y;
  telecommand_message.Emergency_Stop = SN_CTU_read_Emergency_Stop();
  telecommand_message.Arm = 0;
  telecommand_message.Button_A = 0;
  telecommand_message.Button_B = 1;
  telecommand_message.Button_C = 0;
  telecommand_message.Button_D = 1;
  telecommand_message.Encoder_Pos = 1;

  return telecommand_message;
}

uint8_t SN_CTU_get_OBC_Communication_Mode() {
  return SET_OBC_AS_ESPNOW_PEER_CONTROL_MODE;
}

uint8_t SN_CTU_read_Joystick(int axis) {
  // test joystick values, to be replaced by actual functionality to read joystick values from analog pins via ADS1115 ADC
  if (axis == JOYSTICK_X_AXIS) {
    uint8_t joystick_x = joystick_test_x++; 
    if(joystick_test_x > 255) {
      joystick_test_x = 0;
    }
    return joystick_x;
  } else if (axis == JOYSTICK_Y_AXIS) {
    uint8_t joystick_y = joystick_test_y++;
    if(joystick_test_y > 255) {
      joystick_test_y = 0;
    }
    return joystick_y;
  }
  return 0;
}

uint8_t SN_CTU_read_Emergency_Stop() {
  return EMERGENCY_STOP_ACTIVE;
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

void SN_OBC_Handler(){
  // OBC Handler
  // Serial.println("OBC Handler");
  SN_ESPNOW_SendTelemetry();
  // delay(1000);
}


#endif
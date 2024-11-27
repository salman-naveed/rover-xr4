#include <Arduino.h>
#include <SN_Handler.h>
#include <SN_ESPNOW.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
#include <XR4_RCU_Telecommands.h>
#define JOYSTICK_X_AXIS 0
#define JOYSTICK_Y_AXIS 1

extern struct_telecommand_message telecommand_message;

uint8_t EMERGENCY_STOP_ACTIVE = 0;
uint8_t joystick_test_x = 0;
uint8_t joystick_test_y = 0;

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
extern struct_telemetry_message telemetry_message;

#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_RCU_ESP32
void SN_RCU_Handler(){
  // RCU Handler
  SN_RCU_compile_Telecommand_Message();

  SN_ESPNOW_SendTelecommand();

  delay(1000);
}

void SN_RCU_compile_Telecommand_Message() {
  telecommand_message.Command = MSG_DATA_SW_UPGRADE_SETUP_ESP;
  telecommand_message.Comm_Mode = SN_RCU_get_OBC_Communication_Mode();
  telecommand_message.Joystick_X = SN_RCU_read_Joystick(JOYSTICK_X_AXIS);
  telecommand_message.Joystick_Y = SN_RCU_read_Joystick(JOYSTICK_Y_AXIS);
  telecommand_message.Emergency_Stop = SN_RCU_read_Emergency_Stop();
  telecommand_message.Arm = 0;
  telecommand_message.Button_A = 0;
  telecommand_message.Button_B = 1;
  telecommand_message.Button_C = 0;
  telecommand_message.Button_D = 1;
  telecommand_message.Encoder_Pos = 1;
}

uint8_t SN_RCU_get_OBC_Communication_Mode() {
  return SET_OBC_AS_ESPNOW_PEER_CONTROL_MODE;
}

uint8_t SN_RCU_read_Joystick(int axis) {
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

uint8_t SN_RCU_read_Emergency_Stop() {
  return EMERGENCY_STOP_ACTIVE;
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

void SN_OBC_Handler(){
  // OBC Handler
  // Serial.println("OBC Handler");
  // SN_ESPNOW_SendTelemetry();
  // delay(1000);
}


#endif
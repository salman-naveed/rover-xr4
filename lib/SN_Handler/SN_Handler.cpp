#include <Arduino.h>
#include <SN_Handler.h>
#include <SN_ESPNOW.h>
#include <SN_Common.h>
#include <SN_Logger.h>
#include <SN_Joystick.h>
#include <SN_XR_Board_Types.h>

#include <stdint.h>
#include <stdbool.h>

extern xr4_system_context_t xr4_system_context;


extern bool EMERGENCY_STOP_ACTIVE;
extern bool ARMED;
extern bool HEADLIGHTS_ON;

extern bool ERROR_EVENT;

// Function to set a specific flag in the variable
void set_flag(uint16_t *flags, uint8_t bit_position, bool value) {
    if (value) {
        *flags |= (1 << bit_position); // Set the bit
    } else {
        *flags &= ~(1 << bit_position); // Clear the bit
    }
}

// Function to get the value of a specific flag
bool get_flag(uint16_t flags, uint8_t bit_position) {
    return (flags & (1 << bit_position)) != 0;
}

//  ----------------- CTU Handler -----------------

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

void SN_CTU_MainHandler(){
  // CTU Handler
  SN_CTU_compile_Telecommand_Message();

  SN_ESPNOW_SendTelecommand();

}

uint8_t SN_CTU_get_OBC_Communication_Mode() {
  return SET_OBC_AS_ESPNOW_PEER_CONTROL_MODE;
}


uint8_t SN_CTU_read_Emergency_Stop() {
  return EMERGENCY_STOP_ACTIVE;
}
// --------------------------------------------

// ---------------- OBC Handler ----------------

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32



void SN_OBC_TurnOnHeadlights() {
  xr4_system_context.Headlights_On = 1;

  // add GPIO control code here for headlights
}

void SN_OBC_TurnOffHeadlights() {
  xr4_system_context.Headlights_On = 0;

  // add GPIO control code here for headlights
}

void SN_OBC_ExecuteCommands() {
  // Execute HIGH PRIORITY commands received from CTU
  if(xr4_system_context.Emergency_Stop == 1 && xr4_system_context.Armed == 0) {
    EMERGENCY_STOP_ACTIVE = true;
    ARMED = false;
  } else if(xr4_system_context.Emergency_Stop == 1 && xr4_system_context.Armed == 1) {
    EMERGENCY_STOP_ACTIVE = true;
    ARMED = false;
  } else if(xr4_system_context.Emergency_Stop == 0 && xr4_system_context.Armed == 1) {
    EMERGENCY_STOP_ACTIVE = false;
    ARMED = true;
  } else if(xr4_system_context.Emergency_Stop == 0 && xr4_system_context.Armed == 0) {
    EMERGENCY_STOP_ACTIVE = false;
    ARMED = false;
  } else {
    ERROR_EVENT = true;
  }

  // Execute LOW PRIORITY commands received from CTU
  if(xr4_system_context.Headlights_On == 1) {
    SN_OBC_TurnOnHeadlights();
  } else if(xr4_system_context.Headlights_On == 0) {
    SN_OBC_TurnOffHeadlights();
  }

  // add handling for COMMAND & COMM_MODE

}

void SN_OBC_DrivingHandler() {

  JoystickReceivedValues_t joystick_values;

  joystick_values = SN_Joystick_OBC_MapADCValues(xr4_system_context.Joystick_X, xr4_system_context.Joystick_Y);

}


void SN_OBC_MainHandler(){
  // OBC Handler
  SN_OBC_ExecuteCommands();


  SN_OBC_DrivingHandler();


  SN_ESPNOW_SendTelemetry();
}


#endif
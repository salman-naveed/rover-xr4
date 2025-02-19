#include <Arduino.h>
#include <SN_Handler.h>
#include <SN_ESPNOW.h>
#include <SN_Common.h>
#include <SN_Logger.h>
#include <SN_Joystick.h>
#include <SN_XR_Board_Types.h>
#include <SN_Motors.h>

#include <stdint.h>
#include <stdbool.h>

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

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

extern bool TM_received_rdy_to_copy;

extern telecommand_data_t CTU_out_telecommand_data;

uint8_t EMERGENCY_STOP_ACTIVE = 0;
uint8_t joystick_test_x = 0;
uint8_t joystick_test_y = 0;

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

extern telecommand_data_t OBC_in_telecommand_data;
extern bool OBC_TC_received_data_ready;
extern uint8_t OBC_TC_last_received_data_type;

#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

void SN_CTU_MainHandler(){
  // CTU Handler
  SN_CTU_compile_telecommand_data();

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

  // if(joystick_values.joystick_x_mapped_val > 0) {
  //   if(joystick_values.joystick_y_mapped_val == 0){
  //     // Drive Forward
  //     float speed = (float)joystick_values.joystick_x_mapped_val;
  //     SN_Motors_DriveForward(speed);
  //   }
  //   else if(joystick_values.joystick_y_mapped_val > 0){
  //     // Turn Right
  //     float speed = (float)joystick_values.joystick_y_mapped_val;
  //     SN_Motors_TurnRight(speed);
  //   }
  //   else if(joystick_values.joystick_y_mapped_val < 0){
  //     // Turn Left
  //     float speed = (float)joystick_values.joystick_y_mapped_val;
  //     SN_Motors_TurnLeft(speed);
  //   }

  //   // Drive Forward
  //   SN_Motors_DriveForward(speed);
  // } 
  // else if(joystick_values.joystick_x_mapped_val < 0) {
  //   // Drive Backward
  //   float speed = (float)joystick_values.joystick_x_mapped_val;
  //   SN_Motors_DriveBackward(speed);
  // } 
  // else {
  //   // Stop
  //   SN_Motors_Stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
  // }

}


void SN_OBC_MainHandler(){
  // OBC Handler

  SN_Telecommand_updateContext(OBC_in_telecommand_data);

  SN_OBC_ExecuteCommands();

  SN_OBC_DrivingHandler();

  SN_Telemetry_updateStruct(xr4_system_context);

  SN_ESPNOW_SendTelemetry();
}


#endif
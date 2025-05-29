#include <Arduino.h>
#include <SN_Handler.h>
#include <SN_ESPNOW.h>
// #include <SN_Common.h>
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

extern telecommand_data_t CTU_out_telecommand_data;
extern bool CTU_TC_received_data_ready;
extern uint8_t CTU_TM_last_received_data_type;

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
  SN_Telemetry_updateContext(CTU_TM_last_received_data_type);

  SN_CTU_ControlInputsHandler();

  SN_Telecommand_updateStruct(xr4_system_context);

  

  SN_ESPNOW_SendTelecommand(TC_C2_DATA_MSG);

}

void SN_CTU_ControlInputsHandler(){

  JoystickRawADCValues_t CTU_joystick_raw_adc_values = SN_Joystick_ReadRawADCValues();

  CTU_InputStates_t CTU_input_states = SN_CTU_ReadInputStates();

  





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
  if(EMERGENCY_STOP_ACTIVE || !ARMED) {
    // Stop the motors if emergency stop is active
    SN_Motors_Drive(0, 0);
    return;
  }
  else if(ARMED) {
    // Map joystick values to motor speeds
    // Joystick values are in the range of 0-1023, map them to -100 to 100
    JoystickMappedValues_t joystick_values;

    joystick_values = SN_Joystick_OBC_MapADCValues(xr4_system_context.Joystick_X, xr4_system_context.Joystick_Y);

    SN_Motors_Drive(joystick_values.joystick_x_mapped_val, joystick_values.joystick_y_mapped_val);
    return;
  }
}

// OBC Handler
void SN_OBC_MainHandler(){

  // Update OBC context with received telecommand data, which is received from CTU and assigned to OBC_in_telecommand_data using OnTelecommandReceive() callback
  SN_Telecommand_updateContext(OBC_in_telecommand_data); 

  // Execute telecommands received from CTU
  SN_OBC_ExecuteCommands();

  // Execute driving commands received from CTU
  SN_OBC_DrivingHandler();

  // Update outgoing telemetry data struct using the updated context
  SN_Telemetry_updateStruct(xr4_system_context);

  // Send telemetry data to CTU via ESP-NOW
  SN_ESPNOW_SendTelemetry();
}


#endif
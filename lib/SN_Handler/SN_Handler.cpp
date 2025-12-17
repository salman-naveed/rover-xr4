#include <Arduino.h>
#include <SN_Handler.h>
#include <SN_StatusPanel.h>
#include <SN_ESPNOW.h>
// #include <SN_Common.h>
#include <SN_Logger.h>
#include <SN_Joystick.h>
#include <SN_XR_Board_Types.h>
#include <SN_Motors.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
#include <SN_Switches.h>
#include <SN_LCD.h>
#endif

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
  // Update switch states (includes debouncing)
  SN_Switches_Update();
  
  // CTU Handler
  SN_Telemetry_updateContext(CTU_TM_last_received_data_type);

  SN_CTU_ControlInputsHandler();

  SN_Telecommand_updateStruct(xr4_system_context);

  SN_ESPNOW_SendTelecommand(TC_C2_DATA_MSG);
  
  // Update LCD display (non-blocking, updates at configured interval)
  SN_LCD_Update(&xr4_system_context);

}

void SN_CTU_ControlInputsHandler(){

  JoystickRawADCValues_t CTU_joystick_raw_adc_values = SN_Joystick_ReadRawADCValues();

  // Get switch states (debounced and interrupt-driven)
  SwitchStates_t switch_states = SN_Switches_GetStates();

  // Update the XR4 system context with the joystick values
  xr4_system_context.Joystick_X = CTU_joystick_raw_adc_values.joystick_x_raw_val;
  xr4_system_context.Joystick_Y = CTU_joystick_raw_adc_values.joystick_y_raw_val;

  // Update the XR4 system context with the control inputs from new switch handler
  // NOTE: E-STOP is handled by hardware interrupt (ISR) - do NOT overwrite it here!
  // xr4_system_context.Emergency_Stop = switch_states.estop_switch;  // REMOVED - ISR updates this directly
  xr4_system_context.Armed = switch_states.arm_switch;
  xr4_system_context.Headlights_On = switch_states.headlights_switch;
  
  // Update encoder position
  xr4_system_context.Encoder_Pos = (uint16_t)switch_states.encoder_position;
  
  // ========================================
  // LCD Navigation Using Rotary Encoder
  // ========================================
  // Navigate LCD pages using encoder delta (CW = next page, CCW = previous page)
  if (switch_states.encoder_delta > 0) {
    // Clockwise rotation - next page
    SN_LCD_NextPage();
  } else if (switch_states.encoder_delta < 0) {
    // Counter-clockwise rotation - previous page
    SN_LCD_PrevPage();
  }
  
  // Rotary button press - currently unused, can be used for future features
  // (e.g., select menu items, reset encoder, toggle backlight, etc.)
  if (switch_states.rotary_pressed) {
    // Future feature: Enter/select current page option
    logMessage(true, "SN_CTU_ControlInputsHandler", "Rotary button pressed on page %d", SN_LCD_GetCurrentPage());
  }
  
  // ========================================
  // Legacy Button Inputs (Optional)
  // ========================================
  // Note: The old SN_CTU_ReadInputStates() function has been removed to eliminate
  // GPIO conflicts with the new SN_Switches system.
  // 
  // If you have physical buttons wired to GPIO 25, 26, 27:
  // 1. Read them directly: digitalRead(button_a_pin), etc.
  // 2. Or integrate into SN_Switches library for debouncing
  // 
  // For now, setting these to false (no buttons connected)
  xr4_system_context.Buzzer = false;     // No buzzer switch in current design
  xr4_system_context.Button_A = false;   // Set to true if GPIO 25 has button wired
  xr4_system_context.Button_B = false;   // Set to true if GPIO 26 has button wired
  xr4_system_context.Button_C = false;   // Set to true if GPIO 27 has button wired
  xr4_system_context.Button_D = false;   // GPIO 14 now used for E-STOP, not button
  
  // CTU RSSI is measured at receiver (OBC) side, not at transmitter
  xr4_system_context.CTU_RSSI = 0;  // OBC will measure this when receiving telecommand
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

void SN_OBC_ExecuteCommands() {
  // Execute HIGH PRIORITY commands received from CTU
  if (xr4_system_context.Emergency_Stop) {
    xr4_system_context.system_state = XR4_STATE_EMERGENCY_STOP;
  } else if (xr4_system_context.Armed && xr4_system_context.system_state != XR4_STATE_ARMED) {
    logMessage(true, "SN_OBC_ExecuteCommands", "Rover ARMED. Switching to ARMED state.");
    xr4_system_context.system_state = XR4_STATE_ARMED;
  } else if (!xr4_system_context.Armed && xr4_system_context.system_state != XR4_STATE_WAITING_FOR_ARM) {
    xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
  }

  // Execute LOW PRIORITY commands received from CTU
  SN_StatusPanel__ControlHeadlights(xr4_system_context.Headlights_On);
  // add handling for COMMAND & COMM_MODE
}

void SN_OBC_DrivingHandler() {
  if(EMERGENCY_STOP_ACTIVE || !ARMED) {
    // Stop the motors if emergency stop is active
    SN_Motors_Drive(0, 0);
    return;
  }
  // Drive the motors based on joystick values
  JoystickMappedValues_t joystick_values;
  joystick_values = SN_Joystick_OBC_MapADCValues(xr4_system_context.Joystick_X, xr4_system_context.Joystick_Y);
  SN_Motors_Drive(joystick_values.joystick_x_mapped_val, joystick_values.joystick_y_mapped_val);

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
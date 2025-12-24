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
#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
#include <SN_Sensors.h>
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
  // Fast path: check ESTOP/ARM first
  if(xr4_system_context.Emergency_Stop || !xr4_system_context.Armed) {
    SN_Motors_Drive(0, 0);
    return;
  }
  
  // PERFORMANCE OPTIMIZED: Inline joystick mapping to avoid function call overhead
  // X-axis (Forward/Backward) with deadband
  int16_t throttle;
  int16_t raw_x = (int16_t)xr4_system_context.Joystick_X - 2117;  // Center around neutral
  if (raw_x > -50 && raw_x < 50) {
    throttle = 0;  // Deadband
  } else {
    // Fast map: (raw - min) * (out_max - out_min) / (in_max - in_min) + out_min
    throttle = (int16_t)(((int32_t)xr4_system_context.Joystick_X * 200) / 4095) - 100;
    if (throttle > 100) throttle = 100;
    if (throttle < -100) throttle = -100;
  }
  
  // Y-axis (Left/Right steering) with deadband
  int16_t steering;
  int16_t raw_y = (int16_t)xr4_system_context.Joystick_Y - 2000;  // Center around neutral
  if (raw_y > -50 && raw_y < 50) {
    steering = 0;  // Deadband
  } else {
    steering = (int16_t)(((int32_t)xr4_system_context.Joystick_Y * 200) / 4095) - 100;
    if (steering > 100) steering = 100;
    if (steering < -100) steering = -100;
  }
  
  // STEERING DIRECTION FIX:
  // If joystick LEFT makes left motors go forward (wrong direction),
  // uncomment the next line to invert steering:
  // steering = -steering;
  
  // Differential drive calculation (optimized with single clamp operation)
  // Steering inverted: right stick = right turn
  int16_t leftSpeed = throttle - steering;
  int16_t rightSpeed = throttle + steering;
  
  // Clamp both in single operation
  if (leftSpeed > 100) leftSpeed = 100;
  else if (leftSpeed < -100) leftSpeed = -100;
  if (rightSpeed > 100) rightSpeed = 100;
  else if (rightSpeed < -100) rightSpeed = -100;
  
  SN_Motors_Drive(leftSpeed, rightSpeed);
}

void SN_OBC_ReadSensors() {
  // Throttle sensor reading to avoid blocking - read every 250ms
  static unsigned long lastSensorRead = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastSensorRead < 250) {
    return; // Skip this iteration
  }
  lastSensorRead = currentTime;
  
  // Read ADC sensors (battery voltage, current, etc.)
  xr4_system_context.Main_Bus_V = SN_Sensors_ADCGetParameterValue(ADC_CHANN_MAIN_BUS_VOLTAGE);
  xr4_system_context.Main_Bus_I = SN_Sensors_ADCGetParameterValue(ADC_CHANN_MAIN_BUS_CURRENT);
  
  // Read temperature sensor (can be slow)
  xr4_system_context.temp = SN_Sensors_GetBatteryTemperature();
  
  // TODO: Add IMU reading here when SN_USE_IMU is enabled
  // For now, initialize IMU values to zero if not implemented
  // xr4_system_context.Gyro_X = 0.0;
  // xr4_system_context.Gyro_Y = 0.0;
  // xr4_system_context.Gyro_Z = 0.0;
  // xr4_system_context.Acc_X = 0.0;
  // xr4_system_context.Acc_Y = 0.0;
  // xr4_system_context.Acc_Z = 0.0;
  
  // RSSI is updated in ESP-NOW receive callback
  // GPS is updated independently via ticker in SN_GPS module
}

// OBC Handler
void SN_OBC_MainHandler(){
  // Watchdog: Check if we're receiving telecommands
  static unsigned long lastTelecommandTime = 0;
  static bool watchdogActive = false;
  
  if (OBC_TC_received_data_ready) {
    lastTelecommandTime = millis();
    watchdogActive = false;
  }
  
  // If no telecommand for 2 seconds, enter safe mode
  if (millis() - lastTelecommandTime > 2000 && lastTelecommandTime > 0) {
    if (!watchdogActive) {
      watchdogActive = true;
    }
    SN_Motors_Stop(); // Safety: stop motors if no communication
  }

  // Update OBC context with received telecommand data, which is received from CTU and assigned to OBC_in_telecommand_data using OnTelecommandReceive() callback
  SN_Telecommand_updateContext(OBC_in_telecommand_data); 

  // Execute telecommands received from CTU
  SN_OBC_ExecuteCommands();

  // Execute driving commands received from CTU (only if not in watchdog safe mode)
  if (!watchdogActive) {
    SN_OBC_DrivingHandler();
  }

  // Read sensors and update context
  SN_OBC_ReadSensors();

  // Update outgoing telemetry data struct using the updated context
  SN_Telemetry_updateStruct(xr4_system_context);

  // Send telemetry data to CTU via ESP-NOW
  SN_ESPNOW_SendTelemetry();
}


#endif
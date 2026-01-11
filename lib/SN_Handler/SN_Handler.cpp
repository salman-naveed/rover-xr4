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

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
// ============================================================================
// ZERO-LATENCY SENSOR READING - FREERTOS BACKGROUND TASK
// ============================================================================
// This task runs independently in the background, reading IMU/MAG sensors
// without blocking motor control or ESP-NOW communication.
// Priority: 1 (lower than main loop, higher than idle)
// Core: 0 (separate from main loop on dual-core ESP32)
// ============================================================================

// FreeRTOS task handle
static TaskHandle_t sensorTaskHandle = NULL;

// Background sensor reading task
void sensorReadingTask(void *parameter) {
  logMessage(false, "SensorTask", "Background sensor task started on core %d", xPortGetCoreID());
  
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(200); // 5Hz update rate (200ms) - Reduced for lower latency
  
  while (true) {
    // Wait for the next cycle (prevents tight loop)
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    
    // ========================================================================
    // READ ALL I2C SENSORS HERE TO AVOID BUS CONTENTION
    // All I2C devices (ADC, IMU, MAG, temp) accessed from same core (Core 0)
    // This eliminates dual-core I2C bus conflicts
    // ========================================================================
    
    // Read ADC sensors (ADS1115 on I2C) - ONLY if enabled
    #if SN_USE_ADC == 1
    xr4_system_context.Main_Bus_V = SN_Sensors_ADCGetParameterValue(ADC_CHANN_BUS_VOLTAGE_MAIN);
    xr4_system_context.Main_Bus_I = SN_Sensors_ADCGetParameterValue(ADC_CHANN_BUS_CURRENT_MAIN);
    xr4_system_context.Bus_5V = SN_Sensors_ADCGetParameterValue(ADC_CHANN_BUS_VOLTAGE_5V);
    xr4_system_context.Bus_3V3 = SN_Sensors_ADCGetParameterValue(ADC_CHANN_BUS_VOLTAGE_3V3);
    #else
    xr4_system_context.Main_Bus_V = 0.0;
    xr4_system_context.Main_Bus_I = 0.0;
    xr4_system_context.Bus_5V = 0.0;
    xr4_system_context.Bus_3V3 = 0.0;
    #endif
    
    // Read temperature sensor (DS18B20 on OneWire, not I2C but included for consistency)
    xr4_system_context.temp = SN_Sensors_GetBatteryTemperature();
    
    // Read IMU and compute orientation data
    #if SN_USE_IMU == 1
    read_MPU();  // Update MPU sensor readings (MPU6050 on I2C)
    
    // Extract pitch and roll angles (in degrees) from MPU
    float pitch_deg = mpu_sensor.B_angle;  // Pitch (nose up/down)
    float roll_deg = mpu_sensor.C_angle;   // Roll (left/right tilt)
    
    // Store pitch and roll in system context (atomic write for float is safe on ESP32)
    xr4_system_context.Pitch_Degrees = pitch_deg;
    xr4_system_context.Roll_Degrees = roll_deg;
    
    #if SN_USE_MAGNETOMETER == 1
      // Read magnetometer data (QMC5883L on I2C)
      read_MAG();
      
      // Convert pitch/roll to radians for tilt compensation
      float pitch_rad = pitch_deg * PI / 180.0;
      float roll_rad = roll_deg * PI / 180.0;
      
      // Compute tilt-compensated heading using IMU + Magnetometer fusion
      float tilt_comp_heading = computeTiltCompensatedHeading(
        (float)mag_sensor.mag_x, 
        (float)mag_sensor.mag_y, 
        (float)mag_sensor.mag_z,
        pitch_rad,
        roll_rad
      );
      
      xr4_system_context.Heading_Degrees = tilt_comp_heading;
      
      // Copy cardinal direction (e.g., "N", "NE", "E")
      // Note: strncpy is not strictly thread-safe, but 2-byte copy is atomic enough
      strncpy(xr4_system_context.Heading_Cardinal, mag_sensor.direction, 2);
      xr4_system_context.Heading_Cardinal[2] = '\0';
    #else
      // No magnetometer - use yaw from IMU only (will drift over time)
      xr4_system_context.Heading_Degrees = mpu_sensor.A_angle;  // Yaw
      strncpy(xr4_system_context.Heading_Cardinal, "?", 2);
    #endif
    
    #else
    // IMU not enabled - initialize orientation to zero
    xr4_system_context.Heading_Degrees = 0.0;
    strncpy(xr4_system_context.Heading_Cardinal, "N", 2);
    xr4_system_context.Pitch_Degrees = 0.0;
    xr4_system_context.Roll_Degrees = 0.0;
    #endif // SN_USE_IMU
  }
}

// Start the background sensor reading task
void SN_OBC_StartBackgroundSensorTask() {
  // Only create task if at least one sensor is enabled
  // This avoids unnecessary FreeRTOS task switching overhead
  #if (SN_USE_ADC == 1) || (SN_USE_TEMPERATURE_SENSOR == 1) || (SN_USE_IMU == 1) || (SN_USE_MAGNETOMETER == 1)
  
  // Create task to avoid I2C bus contention
  // All I2C devices (ADC, IMU, MAG) must be accessed from same core
  // Create task with:
  // - Priority 0 (idle priority, absolutely lowest, never interrupts main loop)
  // - 4KB stack (enough for sensor calculations)
  // - Core 0 (separate from main loop on Core 1)
  BaseType_t result = xTaskCreatePinnedToCore(
    sensorReadingTask,       // Task function
    "SensorTask",            // Name
    4096,                    // Stack size (bytes)
    NULL,                    // Parameters
    0,                       // Priority (0 = idle priority, absolutely lowest)
    &sensorTaskHandle,       // Task handle
    0                        // Core 0 (main loop typically runs on core 1)
  );
  
  if (result == pdPASS) {
    logMessage(false, "SensorTask", "Background sensor task created on Core 0");
  } else {
    logMessage(false, "SensorTask", "Failed to create background sensor task!");
  }
  
  #else
  logMessage(false, "SensorTask", "All sensors disabled - background task not created");
  #endif
}
#endif // SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


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

  // Telecommand rate limiting (50Hz = 20ms interval)
  // Prevents ESP-NOW channel flooding and maintains low latency
  static unsigned long last_tc_send_time = 0;
  const unsigned long TC_SEND_INTERVAL_MS = 20;  // 50Hz max rate
  
  if (millis() - last_tc_send_time >= TC_SEND_INTERVAL_MS) {
    SN_ESPNOW_SendTelecommand(TC_C2_DATA_MSG);
    last_tc_send_time = millis();
  }
  
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

void 
SN_OBC_DrivingHandler() {
  if(!xr4_system_context.Emergency_Stop && xr4_system_context.Armed) {
    // Inline joystick mapping for zero-latency motor response
    // X-axis is Horizontal Axis of Joystick i.e. Left/Right (steering)
    // Y-axis is Vertical Joystick axis i.e. Forward/Backward (throttle)
    int16_t raw_x = (int16_t)OBC_in_telecommand_data.Joystick_X - 2117;
    int16_t steering = (abs(raw_x) < 50) ? 0 : (int16_t)(((int32_t)OBC_in_telecommand_data.Joystick_X * 200) / 4095) - 100;
    
    int16_t raw_y = (int16_t)OBC_in_telecommand_data.Joystick_Y - 2000;
    int16_t throttle = (abs(raw_y) < 50) ? 0 : (int16_t)(((int32_t)OBC_in_telecommand_data.Joystick_Y * 200) / 4095) - 100;
    
  
    // Differential drive calculation
    int16_t leftSpeed = - steering - throttle;
    int16_t rightSpeed = - steering + throttle;
    
    // Clamp to valid range
    if (leftSpeed > 100) leftSpeed = 100;
    else if (leftSpeed < -100) leftSpeed = -100;
    if (rightSpeed > 100) rightSpeed = 100;
    else if (rightSpeed < -100) rightSpeed = -100;
    
    // Drive motors IMMEDIATELY - no waiting for main loop!
    SN_Motors_Drive(leftSpeed, rightSpeed);
  } else {
    // Safety: Stop motors if ESTOP or disarmed
    SN_Motors_Drive(0, 0);
  }
}


void SN_OBC_ReadSensors() {
  // ALL SENSOR READING NOW DONE IN BACKGROUND TASK ON CORE 0
  // This function is intentionally empty to avoid I2C bus contention
  // 
  // Background task (sensorReadingTask on Core 0) reads:
  //   - ADC (ADS1115) - voltage/current
  //   - Temperature (DS18B20) 
  //   - IMU (MPU6050) - orientation
  //   - Magnetometer (QMC5883L) - heading
  //
  // This eliminates dual-core I2C conflicts that cause latency
  // 
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
    //SN_OBC_DrivingHandler();
  }

  // Read sensors and update context
  SN_OBC_ReadSensors();

  // Update outgoing telemetry data struct using the updated context
  SN_Telemetry_updateStruct(xr4_system_context);

  // Send telemetry (has built-in rotation and timing control)
  SN_ESPNOW_SendTelemetry();
}


#endif
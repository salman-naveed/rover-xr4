#include <Arduino.h>
#include <SN_Joystick.h>
#include <SN_XR_Board_Types.h>
#include <SN_Logger.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
#include <Preferences.h>
#endif

// uint16_t joystick_x_adc_val, joystick_y_adc_val; 
float x_volt, y_volt;

// X-axis --> Forward/Backward
// Y-axis --> Left/Right

// ========================================
// Joystick Presets (Factory Defaults)
// ========================================
// Updated neutral values based on actual measurements
constexpr int JOYSTICK_X_NEUTRAL_DEFAULT = 2117;  // Measured neutral for X-axis (Forward/Backward)
constexpr int JOYSTICK_Y_NEUTRAL_DEFAULT = 2000;  // Measured neutral for Y-axis (Left/Right)

constexpr int JOYSTICK_MAX = 4095;
constexpr int JOYSTICK_MIN = 0;

constexpr int JOYSTICK_X_DEADBAND = 10;
constexpr int JOYSTICK_Y_DEADBAND = 10;

// ========================================
// Runtime Calibration Values
// ========================================
#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
static int JOYSTICK_X_NEUTRAL = JOYSTICK_X_NEUTRAL_DEFAULT;
static int JOYSTICK_Y_NEUTRAL = JOYSTICK_Y_NEUTRAL_DEFAULT;

#if JOYSTICK_CALIBRATION_ENABLED
static Preferences joystick_preferences;
#endif
#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
// OBC uses the same neutral values for mapping received joystick data
static const int JOYSTICK_X_NEUTRAL = JOYSTICK_X_NEUTRAL_DEFAULT;
static const int JOYSTICK_Y_NEUTRAL = JOYSTICK_Y_NEUTRAL_DEFAULT;
#endif


#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#ifndef SN_JOYSTICK_H
#define SN_JOYSTICK_H

#include <SN_GPIO_Definitions.h>
#include <SN_Joystick.h>
#include <SN_Logger.h>

#include "soc/gpio_reg.h"  // For GPIO register addresses
#include "driver/gpio.h"   // For gpio_num_t
#include "esp32/rom/gpio.h" // Optional: for extra macros

#define READ_GPIO(gpio_num) (((gpio_num) < 32) ? ((REG_READ(GPIO_IN_REG) >> (gpio_num)) & 1) \
                                               : ((REG_READ(GPIO_IN1_REG) >> ((gpio_num) - 32)) & 1))


// ========================================
// Legacy Input Initialization (DEPRECATED)
// ========================================
// This function is deprecated - switch inputs now managed by SN_Switches library
// Keeping function for backward compatibility but with minimal functionality
void SN_Input_Init() {
    // Initialize only joystick GPIO pins
    pinMode(joystick_x_pin, INPUT);
    pinMode(joystick_y_pin, INPUT);
    
    // Legacy switch pins now managed by SN_Switches - do not initialize here
    // GPIO 4, 14, 19, 23, 32, 33 are handled by SN_Switches_Init()
    
    // Optional: Initialize remaining button pins if physically wired
    pinMode(button_a_pin, INPUT_PULLUP);
    pinMode(button_b_pin, INPUT_PULLUP);
    pinMode(button_c_pin, INPUT_PULLUP);
    // button_d_pin removed - GPIO 14 now used for E-STOP in SN_Switches

    logMessage(true, "SN_Input_Init", "Joystick and legacy button pins initialized");
}

void SN_Joystick_Init(){
    pinMode(joystick_x_pin, INPUT);
    pinMode(joystick_y_pin, INPUT);
    
#if JOYSTICK_CALIBRATION_ENABLED
    // Load calibration from preferences
    SN_Joystick_LoadCalibration();
    logMessage(true, "SN_Joystick_Init", "Joystick Initialized - X_Neutral: %d, Y_Neutral: %d", 
               JOYSTICK_X_NEUTRAL, JOYSTICK_Y_NEUTRAL);
#else
    logMessage(true, "SN_Joystick_Init", "Joystick Initialized (No Calibration)");
#endif
}

// ========================================
// Calibration Functions
// ========================================

#if JOYSTICK_CALIBRATION_ENABLED

void SN_Joystick_LoadCalibration() {
    joystick_preferences.begin("joystick", true);  // Read-only mode
    
    JOYSTICK_X_NEUTRAL = joystick_preferences.getUInt("x_neutral", JOYSTICK_X_NEUTRAL_DEFAULT);
    JOYSTICK_Y_NEUTRAL = joystick_preferences.getUInt("y_neutral", JOYSTICK_Y_NEUTRAL_DEFAULT);
    
    joystick_preferences.end();
    
    logMessage(true, "SN_Joystick_LoadCalibration", 
               "Loaded calibration - X: %d, Y: %d", JOYSTICK_X_NEUTRAL, JOYSTICK_Y_NEUTRAL);
}

void SN_Joystick_Calibrate() {
    logMessage(true, "SN_Joystick_Calibrate", "Starting calibration - center joystick...");
    
    // Take 100 samples over 1 second for accurate calibration
    uint32_t x_sum = 0, y_sum = 0;
    const int samples = 100;
    
    for (int i = 0; i < samples; i++) {
        x_sum += analogRead(joystick_x_pin);
        y_sum += analogRead(joystick_y_pin);
        delay(10);  // 10ms between samples = 1 second total
    }
    
    JOYSTICK_X_NEUTRAL = x_sum / samples;
    JOYSTICK_Y_NEUTRAL = y_sum / samples;
    
    // Save to preferences
    joystick_preferences.begin("joystick", false);  // Read-write mode
    joystick_preferences.putUInt("x_neutral", JOYSTICK_X_NEUTRAL);
    joystick_preferences.putUInt("y_neutral", JOYSTICK_Y_NEUTRAL);
    joystick_preferences.end();
    
    logMessage(true, "SN_Joystick_Calibrate", 
               "Calibration complete - X: %d, Y: %d (saved to NVS)", 
               JOYSTICK_X_NEUTRAL, JOYSTICK_Y_NEUTRAL);
}

void SN_Joystick_ResetCalibration() {
    joystick_preferences.begin("joystick", false);
    joystick_preferences.clear();
    joystick_preferences.end();
    
    JOYSTICK_X_NEUTRAL = JOYSTICK_X_NEUTRAL_DEFAULT;
    JOYSTICK_Y_NEUTRAL = JOYSTICK_Y_NEUTRAL_DEFAULT;
    
    logMessage(true, "SN_Joystick_ResetCalibration", 
               "Reset to factory defaults - X: %d, Y: %d", 
               JOYSTICK_X_NEUTRAL, JOYSTICK_Y_NEUTRAL);
}

void SN_Joystick_GetNeutralValues(uint16_t* x_neutral, uint16_t* y_neutral) {
    if (x_neutral) *x_neutral = JOYSTICK_X_NEUTRAL;
    if (y_neutral) *y_neutral = JOYSTICK_Y_NEUTRAL;
}

#endif // JOYSTICK_CALIBRATION_ENABLED

// ========================================
// ADC Reading with Optional Averaging
// ========================================

/**
 * Read joystick ADC values with optional averaging
 * 
 * Averaging reduces noise but increases read time:
 * - 1 sample:  ~20µs (no averaging)
 * - 4 samples: ~80µs (recommended balance)
 * - 8 samples: ~160µs (maximum smoothing)
 * 
 * Deadband filtering further reduces jitter
 */
JoystickRawADCValues_t SN_Joystick_ReadRawADCValues() {
    JoystickRawADCValues_t values;

#if JOYSTICK_ADC_AVERAGING_ENABLED
    // ADC averaging for noise reduction
    uint32_t x_sum = 0, y_sum = 0;
    
    for (int i = 0; i < JOYSTICK_ADC_SAMPLE_COUNT; i++) {
        x_sum += analogRead(joystick_x_pin);
        y_sum += analogRead(joystick_y_pin);
    }
    
    values.joystick_x_raw_val = x_sum / JOYSTICK_ADC_SAMPLE_COUNT;
    values.joystick_y_raw_val = y_sum / JOYSTICK_ADC_SAMPLE_COUNT;
#else
    // Single sample (fastest, more noise)
    values.joystick_x_raw_val = analogRead(joystick_x_pin);
    values.joystick_y_raw_val = analogRead(joystick_y_pin);
#endif

    // Input validation - ensure ADC values are in valid range
    if (values.joystick_x_raw_val > JOYSTICK_MAX) {
        logMessage(true, "SN_Joystick", "WARNING: X ADC out of range: %d", values.joystick_x_raw_val);
        values.joystick_x_raw_val = JOYSTICK_X_NEUTRAL;
    }
    if (values.joystick_y_raw_val > JOYSTICK_MAX) {
        logMessage(true, "SN_Joystick", "WARNING: Y ADC out of range: %d", values.joystick_y_raw_val);
        values.joystick_y_raw_val = JOYSTICK_Y_NEUTRAL;
    }

    // Apply deadband - prevents jitter near neutral position
    if (abs((int)values.joystick_x_raw_val - JOYSTICK_X_NEUTRAL) < JOYSTICK_X_DEADBAND) {
        values.joystick_x_raw_val = JOYSTICK_X_NEUTRAL;
    }
    if (abs((int)values.joystick_y_raw_val - JOYSTICK_Y_NEUTRAL) < JOYSTICK_Y_DEADBAND) {
        values.joystick_y_raw_val = JOYSTICK_Y_NEUTRAL;
    }

    return values;
}

// ----- Read CTU Input States -----
// DEPRECATED: This function is replaced by SN_Switches library
// All switch inputs (E-STOP, ARM, Headlights, buttons) now use SN_Switches_GetState()
// This function kept for backward compatibility but returns zeroed struct
CTU_InputStates_t SN_CTU_ReadInputStates() {
    CTU_InputStates_t input_states = {};  // Zero-initialize all fields
    
    // Legacy switch reading disabled - use SN_Switches library instead
    // Example: SN_Switches_GetState(SWITCH_ESTOP) for E-STOP
    //          SN_Switches_GetState(SWITCH_ARM) for ARM
    //          SN_Switches_GetState(SWITCH_HEADLIGHTS) for Headlights
    
    // Optional: Read only button pins that are still defined
    input_states.Button_A = READ_GPIO(button_a_pin);
    input_states.Button_B = READ_GPIO(button_b_pin);
    input_states.Button_C = READ_GPIO(button_c_pin);
    
    logMessage(false, "SN_CTU_ReadInputStates", "DEPRECATED - Use SN_Switches library instead");
    
    return input_states;
}

#endif // SN_JOYSTICK_H
#endif // SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

// ----- Joystick Mapping Function (Used on OBC side) -----
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
// Map the raw ADC values to a range of -100 to 100
JoystickMappedValues_t SN_Joystick_OBC_MapADCValues(uint16_t joystick_x_adc_val, uint16_t joystick_y_adc_val) {
    JoystickMappedValues_t mapped;

    // Apply deadband around neutral position
    const int DEADBAND = 50; // ADC units around neutral position
    
    // X-axis (forward/backward) mapping with deadband
    if (abs((int)joystick_x_adc_val - JOYSTICK_X_NEUTRAL) < DEADBAND) {
        mapped.joystick_x_mapped_val = 0;
    } else {
        mapped.joystick_x_mapped_val = map(joystick_x_adc_val, JOYSTICK_MIN, JOYSTICK_MAX, -100, 100);
    }

    // Y-axis (left/right) mapping with deadband
    if (abs((int)joystick_y_adc_val - JOYSTICK_Y_NEUTRAL) < DEADBAND) {
        mapped.joystick_y_mapped_val = 0;
    } else {
        mapped.joystick_y_mapped_val = map(joystick_y_adc_val, JOYSTICK_MIN, JOYSTICK_MAX, -100, 100);
    }

    return mapped;
}
#endif
  
#pragma once
#include <Arduino.h>
#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

// ========================================
// Joystick Configuration
// ========================================

// ADC Averaging Configuration
#define JOYSTICK_ADC_AVERAGING_ENABLED true   // Set to false to disable averaging
#define JOYSTICK_ADC_SAMPLE_COUNT 4           // Number of samples to average (2-8 recommended)

// Calibration
#define JOYSTICK_CALIBRATION_ENABLED true     // Enable calibration support

// ========================================
// Function Prototypes
// ========================================

void SN_Input_Init();
void SN_Joystick_Init();

// Calibration Functions (if JOYSTICK_CALIBRATION_ENABLED)
#if JOYSTICK_CALIBRATION_ENABLED
/**
 * Calibrate joystick neutral position
 * Call this when joystick is centered
 * Stores calibration to NVS preferences
 */
void SN_Joystick_Calibrate();

/**
 * Load calibration from preferences
 * Called automatically during init
 */
void SN_Joystick_LoadCalibration();

/**
 * Reset calibration to factory defaults
 */
void SN_Joystick_ResetCalibration();

/**
 * Get current neutral values
 */
void SN_Joystick_GetNeutralValues(uint16_t* x_neutral, uint16_t* y_neutral);
#endif

// ========================================
// Data Structures
// ========================================

typedef struct JoystickRawADCValues {
    uint16_t joystick_x_raw_val;
    uint16_t joystick_y_raw_val;
} JoystickRawADCValues_t;

typedef struct CTU_InputStates {
    bool Emergency_Stop : 1;
    bool Armed : 1;
    bool Headlights_On : 1;
    bool Buzzer : 1;
    bool Button_A : 1;
    bool Button_B : 1;
    bool Button_C : 1;
    bool Button_D : 1;
} CTU_InputStates_t;

JoystickRawADCValues_t SN_Joystick_ReadRawADCValues();
CTU_InputStates_t SN_CTU_ReadInputStates();

#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

typedef struct JoystickMappedValues {
    int16_t joystick_x_mapped_val = 0;
    int16_t joystick_y_mapped_val = 0;
} JoystickMappedValues_t;

JoystickMappedValues_t SN_Joystick_OBC_MapADCValues(uint16_t joystick_x_adc_val, uint16_t joystick_y_adc_val);

#endif
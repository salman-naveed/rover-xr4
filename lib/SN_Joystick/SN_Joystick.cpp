#include <Arduino.h>
#include <SN_Joystick.h>
#include <SN_XR_Board_Types.h>

// uint16_t joystick_x_adc_val, joystick_y_adc_val; 
float x_volt, y_volt;

// X-axis --> Forward/Backward
// Y-axis --> Left/Right

// Joystick Presets
constexpr int JOYSTICK_X_NEUTRAL = 1856;
constexpr int JOYSTICK_Y_NEUTRAL = 1880;

constexpr int JOYSTICK_MAX = 4095;
constexpr int JOYSTICK_MIN = 0;

constexpr int JOYSTICK_X_DEADBAND = 10;
constexpr int JOYSTICK_Y_DEADBAND = 10;


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


void SN_Input_Init() {
    // Initialize GPIO pins for joystick and CTU inputs
    pinMode(joystick_x_pin, INPUT);
    pinMode(joystick_y_pin, INPUT);
    
    pinMode(emergency_stop_pin, INPUT_PULLUP);
    pinMode(armed_pin, INPUT_PULLUP);
    pinMode(headlights_on_pin, INPUT_PULLUP);
    pinMode(buzzer_pin, INPUT_PULLUP);
    pinMode(button_a_pin, INPUT_PULLUP);
    pinMode(button_b_pin, INPUT_PULLUP);
    pinMode(button_c_pin, INPUT_PULLUP);
    pinMode(button_d_pin, INPUT_PULLUP);

    logMessage(true, "SN_Input_Init", "Input Pins Initialized");
}

void SN_Joystick_Init(){
    pinMode(joystick_x_pin, INPUT);
    pinMode(joystick_y_pin, INPUT);
    logMessage(true, "SN_Joystick_Init", "Joystick Initialized");
}

// ---- READ ADC VALUES ----
// Read the raw ADC values from the joystick
// and apply deadband filtering
// The deadband is the range around the neutral value
// where the joystick is considered to be at rest
// If the joystick is within this range, the raw value is set to the neutral value
// This is to prevent small movements from being detected
// as joystick movements
// The deadband is defined as a constant value
// The neutral value is the value of the joystick when it is at rest
// The raw value is the value of the joystick when it is moved
// The raw value is read from the ADC


// ----- Read Raw ADC Values with Deadband -----
JoystickRawADCValues_t SN_Joystick_ReadRawADCValues() {
    JoystickRawADCValues_t values;

    values.joystick_x_raw_val = analogRead(joystick_x_pin);
    values.joystick_y_raw_val = analogRead(joystick_y_pin);

    // Apply deadband
    if (abs(values.joystick_x_raw_val - JOYSTICK_X_NEUTRAL) < JOYSTICK_X_DEADBAND) {
        values.joystick_x_raw_val = JOYSTICK_X_NEUTRAL;
    }
    if (abs(values.joystick_y_raw_val - JOYSTICK_Y_NEUTRAL) < JOYSTICK_Y_DEADBAND) {
        values.joystick_y_raw_val = JOYSTICK_Y_NEUTRAL;
    }

    return values;
}

// ----- Read CTU Input States -----
CTU_InputStates_t SN_CTU_ReadInputStates() {
    CTU_InputStates_t input_states;

    // Read GPIO32–39 using GPIO_IN1_REG (bitmask is for 0-7 of this block)
    uint32_t gpio_high = REG_READ(GPIO_IN1_REG);
    // Read GPIO0–31 using GPIO_IN_REG
    uint32_t gpio_low  = REG_READ(GPIO_IN_REG);

    input_states.Emergency_Stop   = READ_GPIO(emergency_stop_pin);
    input_states.Armed            = READ_GPIO(armed_pin);
    input_states.Headlights_On    = READ_GPIO(headlights_on_pin);
    input_states.Buzzer           = READ_GPIO(buzzer_pin);
    input_states.Button_A         = READ_GPIO(button_a_pin);
    input_states.Button_B         = READ_GPIO(button_b_pin);
    input_states.Button_C         = READ_GPIO(button_c_pin);
    input_states.Button_D         = READ_GPIO(button_d_pin);

    return input_states;
}

#endif // SN_JOYSTICK_H
#endif // SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

// ----- Joystick Mapping Function (Used on OBC side) -----
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
// Map the raw ADC values to a range of -100 to 100
JoystickMappedValues_t SN_Joystick_OBC_MapADCValues(uint16_t joystick_x_adc_val, uint16_t joystick_y_adc_val) {
    JoystickMappedValues_t mapped;

    mapped.joystick_x_mapped_val = (joystick_x_adc_val == JOYSTICK_X_NEUTRAL)
        ? 0
        : map(joystick_x_adc_val, JOYSTICK_MIN, JOYSTICK_MAX, -100, 100);

    mapped.joystick_y_mapped_val = (joystick_y_adc_val == JOYSTICK_Y_NEUTRAL)
        ? 0
        : map(joystick_y_adc_val, JOYSTICK_MIN, JOYSTICK_MAX, -100, 100);

    return mapped;
}
#endif
  
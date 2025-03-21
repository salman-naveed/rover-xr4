#pragma once
#include <Arduino.h>
#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

void SN_Joystick_Init();

typedef struct JoystickRawADCValues {
    uint16_t joystick_x_raw_val = 0;
    uint16_t joystick_y_raw_val = 0;
} JoystickRawADCValues_t;

JoystickRawADCValues_t SN_Joystick_ReadRawADCValues();

typedef struct CTU_InputStates {
    bool Emergency_Stop: 1 {false};
    bool Armed : 1 {false};
    bool Headlights_On : 1 {false};
    bool Buzzer : 1 {false};
    bool Button_A : 1 {false};
    bool Button_B : 1 {false};
    bool Button_C : 1 {false};
    bool Button_D : 1 {false};
} CTU_InputStates_t;

CTU_InputStates_t SN_CTU_ReadInputStates();

#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

typedef struct JoystickMappedValues {
    int16_t joystick_x_mapped_val = 0;
    int16_t joystick_y_mapped_val = 0;
} JoystickMappedValues_t;

JoystickMappedValues_t SN_Joystick_OBC_MapADCValues(uint16_t joystick_x_adc_val, uint16_t joystick_y_adc_val);

#endif
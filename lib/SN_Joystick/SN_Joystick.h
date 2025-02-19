#pragma once
#include <Arduino.h>
#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

void SN_Joystick_Init();

typedef struct JoystickRawADCValues {
    uint16_t joystick_x_raw_val;
    uint16_t joystick_y_raw_val;
} JoystickRawADCValues_t;

JoystickRawADCValues_t SN_Joystick_ReadRawADCValues();

#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

typedef struct JoystickMappedValues {
    int16_t joystick_x_mapped_val;
    int16_t joystick_y_mapped_val;
} JoystickMappedValues_t;

JoystickMappedValues_t SN_Joystick_OBC_MapADCValues(uint16_t joystick_x_adc_val, uint16_t joystick_y_adc_val);

#endif
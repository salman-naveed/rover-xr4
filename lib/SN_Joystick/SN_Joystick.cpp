#include <Arduino.h>
#include <SN_Joystick.h>
#include <SN_XR_Board_Types.h>

uint16_t joystick_x_adc_val, joystick_y_adc_val; 
float x_volt, y_volt;

// X-axis --> Forward/Backward
// Y-axis --> Left/Right

// Joystick Presets
const int joystick_x_neutral = 1856;
const int joystick_y_neutral = 1880;

const int joystick_max = 4095;
const int joystick_min = 0;

const int joystick_x_deadband = 10;
const int joystick_y_deadband = 10;


#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#ifndef SN_JOYSTICK_H
#define SN_JOYSTICK_H

#include <SN_GPIO_Definitions.h>
#include <SN_Joystick.h>
#include <SN_Logger.h>


void SN_Joystick_Init(){
    pinMode(joystick_x_pin, INPUT);
    pinMode(joystick_y_pin, INPUT);
    logMessage(true, "SN_Joystick_Init", "Joystick Initialized");
}

uint16_t SN_Joystick_ReadRawADCValues(){

    joystick_x_adc_val = analogRead(joystick_x_pin); 
    joystick_y_adc_val = analogRead(joystick_y_pin);

    if(abs(joystick_x_adc_val - joystick_x_neutral) < joystick_x_deadband){
        joystick_x_adc_val = joystick_x_neutral;
    }

    if(abs(joystick_y_adc_val - joystick_y_neutral) < joystick_y_deadband){
        joystick_y_adc_val = joystick_y_neutral;
    }

    return joystick_x_adc_val, joystick_y_adc_val;

}

#endif
#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


JoystickReceivedValues_t SN_Joystick_OBC_MapADCValues(uint16_t joystick_x_adc_val, uint16_t joystick_y_adc_val){
    int16_t joystick_x_mapped_val, joystick_y_mapped_val;

    if(abs(joystick_x_adc_val - joystick_x_neutral) < joystick_x_deadband){
        joystick_x_mapped_val = 0;
    }
    else{
        joystick_x_mapped_val = map(joystick_x_adc_val, joystick_min, joystick_max, -100, 100);
    }

    if(abs(joystick_y_adc_val - joystick_y_neutral) < joystick_y_deadband){
        joystick_y_mapped_val = 0;
    }
    else{
        joystick_y_mapped_val = map(joystick_y_adc_val, joystick_min, joystick_max, -100, 100);
    }

    return {joystick_x_mapped_val, joystick_y_mapped_val};
}

#endif
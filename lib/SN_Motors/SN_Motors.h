#pragma once
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

// ------ Pin Definitions --------

// Motor 1 - Left Front
#define M1_LF_PWM0A_OUT 32   //Set GPIO 32 as PWM0A
#define M1_LF_PWM0B_OUT 33   //Set GPIO 33 as PWM0B

// Motor 2 - Left Rear
#define M2_LR_PWM1A_OUT 25   //Set GPIO 25 as PWM1A
#define M2_LR_PWM1B_OUT 26   //Set GPIO 26 as PWM1B

// Motor 3 - Right Front
#define M3_RF_PWM0A_OUT 27   //Set GPIO 27 as PWM0A
#define M3_RF_PWM0B_OUT 14   //Set GPIO 14 as PWM0B

// Motor 4 - Right Rear
#define M4_RR_PWM1A_OUT 12   //Set GPIO 12 as PWM1A
#define M4_RR_PWM1B_OUT 13   //Set GPIO 13 as PWM1B


// ------ Function Prototypes --------
void SN_Motors__Init();

static void SN_Motors__GPIO_Init(void);
static void SN_Motors_MCPWM_Init(void);

void SN_Motors_DriveForward(float speed);
void SN_Motors_DriveBackward(float speed);
void SN_Motors_Stop();
void SN_Motors_TurnLeft(float speed);
void SN_Motors_TurnRight(float speed);

static void mcpwm_bdc_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
static void mcpwm_bdc_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
static void mcpwm_bdc_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);


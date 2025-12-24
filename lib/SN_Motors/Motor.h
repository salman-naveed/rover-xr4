#pragma once

#include <driver/mcpwm.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "soc/mcpwm_periph.h"


// ------ Pin Definitions --------

// PWM Frequency Configuration for Motor Control
// Lower frequency (1-5kHz) = Higher torque, more power delivery, audible whine
// Medium frequency (10-20kHz) = Balanced performance, mostly silent
// Higher frequency (25-40kHz) = Smooth operation, lower efficiency
// Recommendation: Start with 10kHz for maximum speed, adjust based on performance
#define MOTOR_PWM_FREQUENCY 10000  // Default: 10kHz for high power delivery

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
// CHANGED: Avoiding GPIO12 (dangerous strapping pin for flash voltage)
// Using GPIO15 instead of GPIO12, keeping GPIO13 unchanged
// NOTE: GPIO16/17 are used for UART2 (GPS), so cannot be used
#define M4_RR_PWM1A_OUT 15   //Set GPIO 15 as PWM1A (was GPIO 12)
#define M4_RR_PWM1B_OUT 13   //Set GPIO 13 as PWM1B (unchanged)

// ------ Class Definitions --------

class IMotor {
public:
    virtual void initPWM(uint32_t frequency = MOTOR_PWM_FREQUENCY) = 0;
    virtual void driveForward(float dutyPercent) = 0;
    virtual void driveBackward(float dutyPercent) = 0;
    virtual void stop() = 0;
    virtual ~IMotor() = default;
};

class Motor : public IMotor {
public:
    Motor(mcpwm_unit_t unit, mcpwm_timer_t timer);

    void initPWM(uint32_t frequency = MOTOR_PWM_FREQUENCY) override;
    void driveForward(float dutyPercent) override;
    void driveBackward(float dutyPercent) override;
    void stop() override;

private:
    mcpwm_unit_t unit;
    mcpwm_timer_t timer;
};

class MotorGroup {
public:
    MotorGroup(IMotor& motor1, IMotor& motor2);

    void init(uint32_t frequency = MOTOR_PWM_FREQUENCY);
    void drive(int16_t speedPercent);
    void stop();

    // Acceleration profile
    void driveWithRamp(int16_t targetSpeed, uint32_t stepDelayMs = 10, int16_t stepSize = 5);

private:
    IMotor& m1;
    IMotor& m2;
    int16_t currentSpeed = 0;
};

class MotorGPIO {
public:
    static void init(); // Initializes all motor GPIOs
};
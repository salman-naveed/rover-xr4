#include "Motor.h"
#include <SN_Logger.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

Motor::Motor(mcpwm_unit_t unit, mcpwm_timer_t timer)
    : unit(unit), timer(timer) {}

void Motor::initPWM(uint32_t frequency) {
    mcpwm_config_t config = {
        .frequency = frequency,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER            
    };
    mcpwm_init(unit, timer, &config);
}

void Motor::driveForward(float dutyPercent) {
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
    mcpwm_set_duty(unit, timer, MCPWM_OPR_A, dutyPercent);
    mcpwm_set_duty_type(unit, timer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
}

void Motor::driveBackward(float dutyPercent) {
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
    mcpwm_set_duty(unit, timer, MCPWM_OPR_B, dutyPercent);
    mcpwm_set_duty_type(unit, timer, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
}

void Motor::stop() {
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
}

MotorGroup::MotorGroup(IMotor& motor1, IMotor& motor2)
    : m1(motor1), m2(motor2) {}

void MotorGroup::init(uint32_t frequency) {
    m1.initPWM(frequency);
    m2.initPWM(frequency);
}

void MotorGroup::drive(int16_t speedPercent) {
    if (speedPercent > 0) {
        m1.driveForward(speedPercent);
        m2.driveForward(speedPercent);
    } else if (speedPercent < 0) {
        m1.driveBackward(-speedPercent);
        m2.driveBackward(-speedPercent);
    } else {
        stop();
    }
    currentSpeed = speedPercent;
}

void MotorGroup::driveWithRamp(int16_t targetSpeed, uint32_t stepDelayMs, int16_t stepSize) {
    while (currentSpeed != targetSpeed) {
        if (abs(currentSpeed - targetSpeed) < stepSize) {
            currentSpeed = targetSpeed;
        } else {
            currentSpeed += (targetSpeed > currentSpeed) ? stepSize : -stepSize;
        }
        drive(currentSpeed);
        vTaskDelay(pdMS_TO_TICKS(stepDelayMs));
    }
}

void MotorGroup::stop() {
    m1.stop();
    m2.stop();
    currentSpeed = 0;
}

void MotorGPIO::init() {
    logMessage(true, "MotorGPIO", "Initializing MCPWM GPIOs");

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, M1_LF_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, M1_LF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, M2_LR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, M2_LR_PWM1B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, M3_RF_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, M3_RF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, M4_RR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, M4_RR_PWM1B_OUT);

    logMessage(true, "MotorGPIO", "GPIOs Initialized");
}
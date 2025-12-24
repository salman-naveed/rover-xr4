#include "Motor.h"
#include <SN_Logger.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

Motor::Motor(mcpwm_unit_t unit, mcpwm_timer_t timer)
    : unit(unit), timer(timer) {}

void Motor::initPWM(uint32_t frequency) {
    mcpwm_config_t config = {
        .frequency = frequency,
        .cmpr_a = 0,    // Start with 0% duty cycle
        .cmpr_b = 0,    // Start with 0% duty cycle
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER            
    };
    mcpwm_init(unit, timer, &config);
    
    // Explicitly ensure both outputs are LOW after init
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
    mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
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
    // Clamp speed to valid range [-100, 100]
    if (speedPercent > 100) speedPercent = 100;
    if (speedPercent < -100) speedPercent = -100;
    
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

    // Initialize all motor control pins as OUTPUT and set LOW before MCPWM init
    // This prevents any floating states that could cause motors to run
    pinMode(M1_LF_PWM0A_OUT, OUTPUT);
    pinMode(M1_LF_PWM0B_OUT, OUTPUT);
    pinMode(M2_LR_PWM1A_OUT, OUTPUT);
    pinMode(M2_LR_PWM1B_OUT, OUTPUT);
    pinMode(M3_RF_PWM0A_OUT, OUTPUT);
    pinMode(M3_RF_PWM0B_OUT, OUTPUT);
    pinMode(M4_RR_PWM1A_OUT, OUTPUT);
    pinMode(M4_RR_PWM1B_OUT, OUTPUT);
    
    // Set all pins LOW to ensure motors are stopped
    digitalWrite(M1_LF_PWM0A_OUT, LOW);
    digitalWrite(M1_LF_PWM0B_OUT, LOW);
    digitalWrite(M2_LR_PWM1A_OUT, LOW);
    digitalWrite(M2_LR_PWM1B_OUT, LOW);
    digitalWrite(M3_RF_PWM0A_OUT, LOW);
    digitalWrite(M3_RF_PWM0B_OUT, LOW);
    digitalWrite(M4_RR_PWM1A_OUT, LOW);
    digitalWrite(M4_RR_PWM1B_OUT, LOW);
    
    delay(100); // Allow pins to settle

    // Now initialize MCPWM
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, M1_LF_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, M1_LF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, M2_LR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, M2_LR_PWM1B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, M3_RF_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, M3_RF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, M4_RR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, M4_RR_PWM1B_OUT);

    logMessage(true, "MotorGPIO", "GPIOs Initialized and set to safe state");
}
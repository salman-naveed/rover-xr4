#include "Motor.h"
#include <SN_Motors.h>
#include <SN_Logger.h>

Motor leftFront(MCPWM_UNIT_0, MCPWM_TIMER_0);
Motor leftRear(MCPWM_UNIT_0, MCPWM_TIMER_1);
Motor rightFront(MCPWM_UNIT_1, MCPWM_TIMER_0);
Motor rightRear(MCPWM_UNIT_1, MCPWM_TIMER_1);

MotorGroup leftMotors(leftFront, leftRear);
MotorGroup rightMotors(rightFront, rightRear);

void SN_Motors_Init() {
    MotorGPIO::init();
    leftMotors.init();
    rightMotors.init();
    // Ensure motors start in stopped state
    leftMotors.stop();
    rightMotors.stop();
    logMessage(true, "SN_Motors_Init", "Motors initialized and stopped");
}

void SN_Motors_Drive(int16_t leftSpeed, int16_t rightSpeed) {
    // Direct drive - no logging for maximum responsiveness
    leftMotors.drive(leftSpeed);
    rightMotors.drive(rightSpeed);
}

void SN_Motors_Stop() {
    leftMotors.stop();
    rightMotors.stop();
}
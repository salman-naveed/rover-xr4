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
    logMessage(true, "SN_Motors_Init", "Motors initialized");
}

void SN_Motors_Drive(int16_t leftSpeed, int16_t rightSpeed) {
    leftMotors.driveWithRamp(leftSpeed);
    rightMotors.driveWithRamp(rightSpeed);
}
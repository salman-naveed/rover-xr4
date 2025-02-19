

#include <SN_Motors.h>
#include <SN_Logger.h>
#include <SN_XR_Board_Types.h>

// Motor Control

static void SN_Motors__GPIO_Init(void){
    // Initialize Motor Control

    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, M1_LF_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, M1_LF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, M2_LR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, M1_LF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, M3_RF_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, M3_RF_PWM0B_OUT);

    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, M4_RR_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1B, M4_RR_PWM1B_OUT);


    logMessage(true, "SN_Motors_Init", "Motor Control Initialized");
}


static void SN_Motors_MCPWM_Init(void)
{
    // initial mcpwm configuration
    logMessage(true, "SN_Motors_Init", "Configuring Initial Parameters of mcpwm...");

    mcpwm_config_t pwm_config;
    
    pwm_config.frequency = 20000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);

    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);   
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1, &pwm_config);   
    
}

void SN_Motors__Init()
{
    SN_Motors__GPIO_Init();

    SN_Motors_MCPWM_Init();
}

void SN_Motors_DriveForward(float speed)
{
    mcpwm_bdc_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, speed);
    mcpwm_bdc_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_1, speed);

    mcpwm_bdc_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, speed);
    mcpwm_bdc_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_1, speed);

}

void SN_Motors_DriveBackward(float speed)
{
    mcpwm_bdc_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, speed);
    mcpwm_bdc_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_1, speed);

    mcpwm_bdc_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_0, speed);
    mcpwm_bdc_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_1, speed);

}

void SN_Motors_Stop()
{
    mcpwm_bdc_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    mcpwm_bdc_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_1);

    mcpwm_bdc_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);
    mcpwm_bdc_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_1);
}

void SN_Motors_TurnLeft(float speed)
{
    mcpwm_bdc_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, speed);

}

void SN_Motors_TurnRight(float speed)
{

}






/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void mcpwm_bdc_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void mcpwm_bdc_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void mcpwm_bdc_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}
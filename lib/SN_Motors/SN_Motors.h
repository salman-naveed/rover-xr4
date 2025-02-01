

// Pin Definitions

// #define GPIO_PWM0A_OUT 15   //Set GPIO 15 as PWM0A
// #define GPIO_PWM0B_OUT 16   //Set GPIO 16 as PWM0B

#define MOTOR_A_IN1 32
#define MOTOR_A_IN2 33

#define MOTOR_B_IN1 25
#define MOTOR_B_IN2 26

#define MOTOR_C_IN1 27
#define MOTOR_C_IN2 14

#define MOTOR_D_IN1 12
#define MOTOR_D_IN2 13

#define MOTOR_A_EN 15
#define MOTOR_B_EN 2
#define MOTOR_C_EN 4
#define MOTOR_D_EN 16


static void SN_Motors__GPIO_Init(void);
static void SN_Motors_MCPWM_Init(void);

static void SN_Motors_Drive_Forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
static void SN_Motors_Drive_Backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle);
static void SN_Motors_Stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num);


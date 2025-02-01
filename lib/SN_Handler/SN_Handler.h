
void SN_OBC_MainHandler();

void SN_OBC_ExecuteCommands();

void SN_OBC_TurnOnHeadlights();

void SN_OBC_TurnOffHeadlights();

void SN_OBC_DrivingHandler();

void SN_CTU_Handler();

uint8_t SN_CTU_get_OBC_Communication_Mode();

uint8_t SN_CTU_read_Joystick(int axis);

uint8_t SN_CTU_read_Emergency_Stop();

// Bit positions for the flags
#define EMERGENCY_STOP_BIT 7
#define ARMED_BIT 6
#define BUTTON_A_BIT 5
#define BUTTON_B_BIT 4
#define BUTTON_C_BIT 3
#define BUTTON_D_BIT 2
#define HEADLIGHTS_ON_BIT 1
#define BUZZER_BIT 0

// Function to set a specific flag in the variable
void set_flag(uint16_t *flags, uint8_t bit_position, bool value);

// Function to get the value of a specific flag
bool get_flag(uint16_t flags, uint8_t bit_position);
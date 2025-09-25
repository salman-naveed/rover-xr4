#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#ifdef SN_JOYSTICK_H

// Joystick Control
#define joystick_x_pin A3 // Joystick X-axis input pin GPIO39 (SVN A3) LEFT/RIGHT Horizontal
#define joystick_y_pin A0 // Joystick Y-axis input pin GPIO36 (SVP A0) FWD/REVERSE Vertical

// CTU Input Switches
#define emergency_stop_pin 34 // Emergency Stop Switch input pin GPIO34
#define armed_pin 35 // Armed Switch input pin GPIO35
#define headlights_on_pin 32 // Headlights On Switch input pin GPIO32
#define buzzer_pin 33 // Buzzer Switch input pin GPIO33
#define button_a_pin 25 // Button A input pin GPIO25
#define button_b_pin 26 // Button B input pin GPIO26
#define button_c_pin 27 // Button C input pin GPIO27
#define button_d_pin 14 // Button D input pin GPIO14


#endif


#endif

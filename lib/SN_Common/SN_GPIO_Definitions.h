#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#ifdef SN_JOYSTICK_H

// Joystick Control
#define joystick_x_pin A0 // Joystick X-axis input pin GPIO36 SVP
#define joystick_y_pin A3 // Joystick Y-axis input pin GPIO39 SVN


#endif


#endif

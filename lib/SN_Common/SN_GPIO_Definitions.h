#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


#endif

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#ifdef SN_JOYSTICK_H

// ========================================
// CTU GPIO Pin Assignments
// ========================================

// Joystick Control
#define joystick_x_pin A3 // Joystick X-axis input pin GPIO39 (SVN A3) LEFT/RIGHT Horizontal
#define joystick_y_pin A0 // Joystick Y-axis input pin GPIO36 (SVP A0) FWD/REVERSE Vertical

// ========================================
// Legacy Switch Inputs (DEPRECATED - Now using SN_Switches library)
// ========================================
// The following pins are now managed by SN_Switches:
// - GPIO 4:  ARM switch (moved from GPIO 35)
// - GPIO 14: E-STOP switch (moved from GPIO 34)
// - GPIO 19: HEADLIGHTS switch (moved from GPIO 32)
// - GPIO 32: ROTARY ENCODER CLK (was headlights_on_pin)
// - GPIO 33: ROTARY ENCODER DT (was buzzer_pin)
// - GPIO 23: ROTARY ENCODER BUTTON

// Legacy pin definitions (COMMENTED OUT to prevent conflicts)
// #define emergency_stop_pin 34  // DEPRECATED - Now GPIO 14 in SN_Switches
// #define armed_pin 35           // DEPRECATED - Now GPIO 4 in SN_Switches
// #define headlights_on_pin 32   // DEPRECATED - Now GPIO 19 in SN_Switches (GPIO 32 is encoder CLK)
// #define buzzer_pin 33          // DEPRECATED - GPIO 33 now used for encoder DT
// #define button_d_pin 14        // DEPRECATED - GPIO 14 now used for E-STOP

// Remaining legacy buttons (if still needed - wire to these GPIOs)
#define button_a_pin 25 // Button A input pin GPIO25
#define button_b_pin 26 // Button B input pin GPIO26
#define button_c_pin 27 // Button C input pin GPIO27
// Note: Button D (GPIO 14) removed - pin now used for E-STOP in SN_Switches

// ========================================
// Migration Notes:
// ========================================
// If you need the legacy Emergency_Stop, Armed, Headlights, or Buzzer functionality:
// 1. Use SN_Switches_GetStates() instead of SN_CTU_ReadInputStates()
// 2. Access via SwitchStates_t.estop_switch, .arm_switch, .headlights_switch
// 3. Encoder position available via SwitchStates_t.encoder_position
// 
// The new system provides:
// - Hardware interrupts for E-STOP (< 100µs response)
// - Hardware interrupts for encoder (no missed counts)
// - Proper debouncing (20-30ms)
// - One-shot event flags for state changes


#endif


#endif

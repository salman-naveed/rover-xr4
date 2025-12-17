#pragma once
#include <Arduino.h>
#include <SN_XR_Board_Types.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

// ========================================
// GPIO Pin Definitions for CTU Switches
// ========================================

// Rotary Encoder Pins
#define ROTARY_CLK_PIN    32
#define ROTARY_DT_PIN     33
#define ROTARY_SW_PIN     23

// Control Switch Pins
#define ARM_SWITCH_PIN         4
#define HEADLIGHTS_SWITCH_PIN  19
#define ESTOP_SWITCH_PIN       14

// ========================================
// Debounce Timing Configuration
// ========================================

#define DEBOUNCE_DELAY_MS     30    // Regular switches (ARM, HEADLIGHTS, ROTARY_SW)
#define ESTOP_DEBOUNCE_MS     20    // E-STOP needs faster response
#define ENCODER_DEBOUNCE_MS   2     // Rotary encoder (2ms in ISR using micros)

// ========================================
// Data Structures
// ========================================

// Switch states structure
typedef struct {
    // Switch states (debounced)
    bool arm_switch;
    bool headlights_switch;
    bool estop_switch;
    bool rotary_button;
    
    // Rotary encoder state
    int16_t encoder_position;
    int8_t encoder_delta;  // Change since last read (-1, 0, or +1)
    
    // State flags
    bool estop_triggered;     // Set when E-STOP transitions to triggered
    bool arm_changed;         // Set when ARM switch changes
    bool headlights_changed;  // Set when HEADLIGHTS switch changes
    bool rotary_pressed;      // Set when rotary button is pressed
} SwitchStates_t;

// ========================================
// Function Prototypes
// ========================================

// Initialization
void SN_Switches_Init();

// Main update function (call from main loop)
void SN_Switches_Update();

// Getters
SwitchStates_t SN_Switches_GetStates();
bool SN_Switches_IsEStopActive();
bool SN_Switches_IsArmed();
bool SN_Switches_AreHeadlightsOn();
int16_t SN_Switches_GetEncoderPosition();
int8_t SN_Switches_GetEncoderDelta();

// Reset encoder position
void SN_Switches_ResetEncoder();

// Interrupt service routines (for internal use)
void IRAM_ATTR SN_Switches_EStop_ISR();
void IRAM_ATTR SN_Switches_Encoder_ISR();

#endif // SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

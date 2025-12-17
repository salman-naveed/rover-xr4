#include "SN_Switches.h"

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#include <SN_Logger.h>
#include <SN_Common.h>

extern xr4_system_context_t xr4_system_context;

// ========================================
// Internal State Variables
// ========================================

// Current switch states
static volatile SwitchStates_t switchStates;

// Debouncing variables for regular switches
static unsigned long lastArmDebounceTime = 0;
static unsigned long lastHeadlightsDebounceTime = 0;
static unsigned long lastRotaryButtonDebounceTime = 0;

// Last stable states for edge detection
static bool lastArmState = false;
static bool lastHeadlightsState = false;
static bool lastRotaryButtonState = false;

// Raw (non-debounced) states for debouncing logic
static bool armRawState = false;
static bool headlightsRawState = false;
static bool rotaryButtonRawState = false;

// E-STOP specific variables (interrupt-driven)
static volatile unsigned long lastEStopInterruptTime = 0;
static volatile bool estopRawState = false;

// Rotary encoder specific variables (interrupt-driven)
static volatile int16_t encoderCount = 0;
static volatile unsigned long lastEncoderInterruptTime = 0;
static volatile uint8_t lastEncoderState = 0;

// ========================================
// Interrupt Service Routines
// ========================================
// 
// E-STOP SAFETY ARCHITECTURE:
// ---------------------------
// The E-STOP switch is SAFETY-CRITICAL and uses hardware interrupts for immediate response.
// 
// CRITICAL: To prevent race conditions, the E-STOP state is managed ONLY by the ISR:
//   - ISR updates: switchStates.estop_switch AND xr4_system_context.Emergency_Stop
//   - main.cpp: Does NOT overwrite Emergency_Stop (ISR is authoritative)
//   - Handler: Does NOT overwrite Emergency_Stop (ISR is authoritative)
//
// This ensures the E-STOP state is NEVER overwritten by polling, preventing safety issues.
// Response time: Hardware interrupt < 1us, ISR execution < 100us
//

/**
 * E-STOP Interrupt Handler
 * SAFETY CRITICAL - Must respond immediately
 * Uses hardware interrupt for fastest response
 * 
 * This ISR is triggered immediately when GPIO 14 changes state.
 * Hardware detects the change in < 1us, ISR execution typically < 100us.
 * 
 * IMPORTANT: This uses LEVEL-BASED logic, not edge detection, to prevent
 * getting stuck if an interrupt is missed during debouncing.
 */
void IRAM_ATTR SN_Switches_EStop_ISR() {
    unsigned long currentTime = millis();
    
    // Read current pin state immediately
    bool currentState = digitalRead(ESTOP_SWITCH_PIN);
    
    // E-STOP is wired as NC (Normally Closed)
    // Normal state: LOW (closed to GND)
    // Triggered state: HIGH (open, pulled up)
    
    // Check if state has actually changed (ignore redundant interrupts from same state)
    if (currentState != estopRawState) {
        // State changed - apply debounce timing
        if (currentTime - lastEStopInterruptTime >= ESTOP_DEBOUNCE_MS) {
            // Debounce period has passed - accept the new state
            estopRawState = currentState;
            lastEStopInterruptTime = currentTime;
            
            // LEVEL-BASED: Update to match current pin state
            if (currentState == HIGH) {
                // E-STOP is currently triggered
                switchStates.estop_switch = true;
                switchStates.estop_triggered = true;
                
                // Immediately update system context for safety
                xr4_system_context.Emergency_Stop = true;
            } else {
                // E-STOP is currently released/normal
                switchStates.estop_switch = false;
                
                xr4_system_context.Emergency_Stop = false;
            }
        }
        // else: Debounce timer hasn't expired yet - ignore this transition
    }
    // else: Same state as before - this is just switch bounce, ignore it
}

/**
 * Rotary Encoder Interrupt Handler
 * Tracks encoder rotation using quadrature decoding
 * 
 * Uses improved quadrature decoding with aggressive noise rejection.
 * Mechanical encoders can have significant bounce - we need strict filtering.
 */
void IRAM_ATTR SN_Switches_Encoder_ISR() {
    static unsigned long lastValidTransition = 0;
    static int8_t lastDirection = 0;  // Track last movement direction for hysteresis
    unsigned long currentMicros = micros();
    
    // Very aggressive debounce - mechanical encoders are noisy
    // Most cheap encoders need 2-5ms between valid transitions
    if ((currentMicros - lastValidTransition) < 2000) { // 2ms minimum
        return;
    }
    
    // Read current encoder state
    uint8_t clk = digitalRead(ROTARY_CLK_PIN);
    uint8_t dt = digitalRead(ROTARY_DT_PIN);
    uint8_t currentState = (clk << 1) | dt;
    
    // Only process if state actually changed
    if (currentState == lastEncoderState) {
        return;
    }
    
    // Use simple reliable decoding - only count on CLK falling edge
    // This gives us 1 count per detent (mechanical click position)
    // CW:  When CLK goes from 1->0, if DT=1 then clockwise
    // CCW: When CLK goes from 1->0, if DT=0 then counter-clockwise
    
    uint8_t lastCLK = (lastEncoderState >> 1) & 0x01;
    uint8_t currentCLK = (currentState >> 1) & 0x01;
    
    // Only count on CLK falling edge (1 -> 0)
    if (lastCLK == 1 && currentCLK == 0) {
        uint8_t currentDT = currentState & 0x01;
        int8_t direction;
        
        if (currentDT == 1) {
            direction = 1;   // Clockwise
        } else {
            direction = -1;  // Counter-clockwise
        }
        
        // Simple hysteresis: if direction changed, require longer time
        // This filters out quick back-and-forth jitter
        if (direction != lastDirection && lastDirection != 0) {
            // Direction changed - need extra time to be valid (3ms vs 2ms)
            if ((currentMicros - lastValidTransition) < 3000) {
                lastEncoderState = currentState;
                return;  // Ignore this transition - likely jitter
            }
        }
        
        // Valid transition - update count
        encoderCount += direction;
        switchStates.encoder_delta = direction;
        switchStates.encoder_position = encoderCount;
        lastDirection = direction;
        lastValidTransition = currentMicros;
    }
    
    lastEncoderState = currentState;
}

// ========================================
// Initialization
// ========================================

void SN_Switches_Init() {
    logMessage(true, "SN_Switches_Init", "Initializing CTU switches...");
    
    // Initialize switch states structure
    switchStates.arm_switch = false;
    switchStates.headlights_switch = false;
    switchStates.estop_switch = false;
    switchStates.rotary_button = false;
    switchStates.encoder_position = 0;
    switchStates.encoder_delta = 0;
    switchStates.estop_triggered = false;
    switchStates.arm_changed = false;
    switchStates.headlights_changed = false;
    switchStates.rotary_pressed = false;
    
    // Configure GPIO pins
    // Regular switches with external pullup (10kΩ to 3.3V)
    pinMode(ARM_SWITCH_PIN, INPUT);
    pinMode(HEADLIGHTS_SWITCH_PIN, INPUT);
    
    // E-STOP: NC switch, external pullup, interrupt on CHANGE
    pinMode(ESTOP_SWITCH_PIN, INPUT);
    
    // Rotary encoder with internal pullups
    pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
    pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
    pinMode(ROTARY_SW_PIN, INPUT_PULLUP);
    
    // Read initial states
    armRawState = digitalRead(ARM_SWITCH_PIN);
    lastArmState = armRawState;
    switchStates.arm_switch = armRawState;
    
    headlightsRawState = digitalRead(HEADLIGHTS_SWITCH_PIN);
    lastHeadlightsState = headlightsRawState;
    switchStates.headlights_switch = headlightsRawState;
    
    estopRawState = digitalRead(ESTOP_SWITCH_PIN);
    switchStates.estop_switch = (estopRawState == HIGH); // HIGH = triggered
    
    rotaryButtonRawState = digitalRead(ROTARY_SW_PIN);
    lastRotaryButtonState = rotaryButtonRawState;
    switchStates.rotary_button = (rotaryButtonRawState == LOW); // Active LOW
    
    // Initialize encoder state
    uint8_t clk = digitalRead(ROTARY_CLK_PIN);
    uint8_t dt = digitalRead(ROTARY_DT_PIN);
    lastEncoderState = (clk << 1) | dt;
    encoderCount = 0;
    
    // Attach interrupts
    // E-STOP: CHANGE interrupt for both press and release detection
    attachInterrupt(digitalPinToInterrupt(ESTOP_SWITCH_PIN), 
                    SN_Switches_EStop_ISR, 
                    CHANGE);
    
    // Rotary Encoder: Interrupt on CLK pin changes
    attachInterrupt(digitalPinToInterrupt(ROTARY_CLK_PIN), 
                    SN_Switches_Encoder_ISR, 
                    CHANGE);
    
    logMessage(true, "SN_Switches_Init", "Switches initialized successfully");
    logMessage(false, "SN_Switches_Init", "Initial states - ARM: %d, HEADLIGHTS: %d, E-STOP: %d", 
               switchStates.arm_switch, 
               switchStates.headlights_switch, 
               switchStates.estop_switch);
}

// ========================================
// Main Update Function
// ========================================

/**
 * Update switch states with debouncing
 * Call this from the main loop (not time-critical switches)
 * E-STOP and Encoder are handled by interrupts
 */
void SN_Switches_Update() {
    unsigned long currentTime = millis();
    
    // ===== ARM Switch Debouncing =====
    bool armCurrentReading = digitalRead(ARM_SWITCH_PIN);
    
    if (armCurrentReading != armRawState) {
        // State changed, reset debounce timer
        lastArmDebounceTime = currentTime;
        armRawState = armCurrentReading;
    }
    
    if ((currentTime - lastArmDebounceTime) > DEBOUNCE_DELAY_MS) {
        // Reading has been stable for debounce period
        if (armCurrentReading != lastArmState) {
            // State has actually changed after debouncing
            lastArmState = armCurrentReading;
            switchStates.arm_switch = armCurrentReading;
            switchStates.arm_changed = true;
            
            // Update system context
            xr4_system_context.Armed = armCurrentReading;
            
            logMessage(true, "SN_Switches_Update", "ARM switch %s", 
                      armCurrentReading ? "ARMED" : "DISARMED");
        }
    }
    
    // ===== HEADLIGHTS Switch Debouncing =====
    bool headlightsCurrentReading = digitalRead(HEADLIGHTS_SWITCH_PIN);
    
    if (headlightsCurrentReading != headlightsRawState) {
        lastHeadlightsDebounceTime = currentTime;
        headlightsRawState = headlightsCurrentReading;
    }
    
    if ((currentTime - lastHeadlightsDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (headlightsCurrentReading != lastHeadlightsState) {
            lastHeadlightsState = headlightsCurrentReading;
            switchStates.headlights_switch = headlightsCurrentReading;
            switchStates.headlights_changed = true;
            
            // Update system context
            xr4_system_context.Headlights_On = headlightsCurrentReading;
            
            logMessage(true, "SN_Switches_Update", "HEADLIGHTS %s", 
                      headlightsCurrentReading ? "ON" : "OFF");
        }
    }
    
    // ===== Rotary Button Debouncing =====
    bool rotaryButtonCurrentReading = digitalRead(ROTARY_SW_PIN);
    
    if (rotaryButtonCurrentReading != rotaryButtonRawState) {
        lastRotaryButtonDebounceTime = currentTime;
        rotaryButtonRawState = rotaryButtonCurrentReading;
    }
    
    if ((currentTime - lastRotaryButtonDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (rotaryButtonCurrentReading != lastRotaryButtonState) {
            lastRotaryButtonState = rotaryButtonCurrentReading;
            // Button is active LOW
            bool buttonPressed = (rotaryButtonCurrentReading == LOW);
            switchStates.rotary_button = buttonPressed;
            
            if (buttonPressed) {
                switchStates.rotary_pressed = true;
                logMessage(false, "SN_Switches_Update", "Rotary button pressed");
            }
        }
    }
    
    // Note: E-STOP and encoder are handled by interrupts
    // Check if E-STOP was triggered and needs attention
    if (switchStates.estop_triggered) {
        logMessage(true, "SN_Switches_Update", "*** E-STOP TRIGGERED ***");
        // Flag will be cleared by getter or manually
    }
}

// ========================================
// Getter Functions
// ========================================

SwitchStates_t SN_Switches_GetStates() {
    // Create a copy of current states
    SwitchStates_t currentStates;
    currentStates.arm_switch = switchStates.arm_switch;
    currentStates.headlights_switch = switchStates.headlights_switch;
    currentStates.estop_switch = switchStates.estop_switch;
    currentStates.rotary_button = switchStates.rotary_button;
    currentStates.encoder_position = switchStates.encoder_position;
    currentStates.encoder_delta = switchStates.encoder_delta;
    currentStates.estop_triggered = switchStates.estop_triggered;
    currentStates.arm_changed = switchStates.arm_changed;
    currentStates.headlights_changed = switchStates.headlights_changed;
    currentStates.rotary_pressed = switchStates.rotary_pressed;
    
    // Clear one-shot flags after reading
    switchStates.estop_triggered = false;
    switchStates.arm_changed = false;
    switchStates.headlights_changed = false;
    switchStates.rotary_pressed = false;
    switchStates.encoder_delta = 0;
    
    return currentStates;
}

bool SN_Switches_IsEStopActive() {
    return switchStates.estop_switch;
}

bool SN_Switches_IsArmed() {
    return switchStates.arm_switch;
}

bool SN_Switches_AreHeadlightsOn() {
    return switchStates.headlights_switch;
}

int16_t SN_Switches_GetEncoderPosition() {
    return switchStates.encoder_position;
}

int8_t SN_Switches_GetEncoderDelta() {
    int8_t delta = switchStates.encoder_delta;
    switchStates.encoder_delta = 0;  // Clear after reading
    return delta;
}

void SN_Switches_ResetEncoder() {
    noInterrupts();
    encoderCount = 0;
    switchStates.encoder_position = 0;
    switchStates.encoder_delta = 0;
    interrupts();
    
    logMessage(false, "SN_Switches_ResetEncoder", "Encoder position reset to 0");
}

#endif // SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

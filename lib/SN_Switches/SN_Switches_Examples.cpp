/**
 * SN_Switches Usage Examples
 * 
 * This file demonstrates how to use the CTU switch functionality
 * including ARM, HEADLIGHTS, E-STOP, and Rotary Encoder
 */

#include <Arduino.h>
#include <SN_Switches.h>
#include <SN_Common.h>
#include <SN_Logger.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

extern xr4_system_context_t xr4_system_context;

// ========================================
// Example 1: Basic Switch Monitoring
// ========================================
void example_basic_monitoring() {
    // Get all switch states
    SwitchStates_t states = SN_Switches_GetStates();
    
    // Check individual switches
    if (states.arm_changed) {
        logMessage(true, "Example", "ARM switch changed to: %s", 
                  states.arm_switch ? "ARMED" : "DISARMED");
    }
    
    if (states.headlights_changed) {
        logMessage(true, "Example", "Headlights switched: %s", 
                  states.headlights_switch ? "ON" : "OFF");
    }
    
    if (states.estop_triggered) {
        logMessage(true, "Example", "*** EMERGENCY STOP ACTIVATED ***");
        // Handle emergency stop immediately
    }
}

// ========================================
// Example 2: E-STOP Safety Check
// ========================================
void example_safety_check() {
    // Check E-STOP status before any motor operation
    if (SN_Switches_IsEStopActive()) {
        logMessage(true, "Safety", "Cannot operate - E-STOP is active!");
        // Stop all motors
        // Disable actuators
        return;
    }
    
    // Safe to proceed
    logMessage(false, "Safety", "E-STOP clear - safe to operate");
}

// ========================================
// Example 3: ARM/DISARM Logic Integration
// ========================================
void example_arming_logic() {
    SwitchStates_t states = SN_Switches_GetStates();
    
    if (states.arm_changed) {
        if (states.arm_switch) {
            // Arming sequence
            logMessage(true, "Arming", "Initiating arming sequence...");
            
            // Check pre-arm conditions
            if (SN_Switches_IsEStopActive()) {
                logMessage(true, "Arming", "Cannot arm - E-STOP is active!");
                return;
            }
            
            // Perform arming
            xr4_system_context.Armed = true;
            xr4_system_context.system_state = XR4_STATE_ARMED;
            logMessage(true, "Arming", "System ARMED");
            
        } else {
            // Disarming sequence
            logMessage(true, "Arming", "Disarming system...");
            xr4_system_context.Armed = false;
            xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
            logMessage(true, "Arming", "System DISARMED");
        }
    }
}

// ========================================
// Example 4: Headlights Control
// ========================================
void example_headlights_control() {
    SwitchStates_t states = SN_Switches_GetStates();
    
    if (states.headlights_changed) {
        if (states.headlights_switch) {
            // Turn on headlights
            logMessage(true, "Headlights", "Turning ON headlights");
            // GPIO control or command to OBC
            xr4_system_context.Headlights_On = true;
        } else {
            // Turn off headlights
            logMessage(true, "Headlights", "Turning OFF headlights");
            xr4_system_context.Headlights_On = false;
        }
    }
}

// ========================================
// Example 5: Rotary Encoder for Menu Navigation
// ========================================
void example_encoder_menu_navigation() {
    static int currentMenuItem = 0;
    const int maxMenuItems = 5;
    
    SwitchStates_t states = SN_Switches_GetStates();
    
    // Get encoder changes
    int8_t delta = states.encoder_delta;
    
    if (delta != 0) {
        currentMenuItem += delta;
        
        // Wrap around menu
        if (currentMenuItem < 0) currentMenuItem = maxMenuItems - 1;
        if (currentMenuItem >= maxMenuItems) currentMenuItem = 0;
        
        logMessage(false, "Menu", "Current menu item: %d (delta: %d)", 
                  currentMenuItem, delta);
    }
    
    // Check for button press to select
    if (states.rotary_pressed) {
        logMessage(true, "Menu", "Selected menu item: %d", currentMenuItem);
        // Execute menu action
    }
}

// ========================================
// Example 6: Encoder for Value Adjustment
// ========================================
void example_encoder_value_adjustment() {
    static float motorSpeed = 50.0; // 0-100%
    const float speedIncrement = 5.0;
    
    int8_t delta = SN_Switches_GetEncoderDelta();
    
    if (delta != 0) {
        motorSpeed += (delta * speedIncrement);
        
        // Clamp to valid range
        if (motorSpeed < 0.0) motorSpeed = 0.0;
        if (motorSpeed > 100.0) motorSpeed = 100.0;
        
        logMessage(false, "Speed", "Motor speed adjusted to: %.1f%%", motorSpeed);
    }
}

// ========================================
// Example 7: Complete Safety Loop
// ========================================
void example_complete_safety_loop() {
    // This shows a complete safety check loop
    
    // 1. Check E-STOP first (highest priority)
    if (SN_Switches_IsEStopActive()) {
        logMessage(true, "Safety Loop", "E-STOP ACTIVE - All operations halted");
        xr4_system_context.Emergency_Stop = true;
        xr4_system_context.system_state = XR4_STATE_EMERGENCY_STOP;
        // Stop all motors immediately
        return; // Exit immediately
    }
    
    // 2. Check ARM state
    if (!SN_Switches_IsArmed()) {
        logMessage(false, "Safety Loop", "System not armed - standby mode");
        xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
        // Only allow non-critical operations
        return;
    }
    
    // 3. System is armed and E-STOP is clear - normal operation
    logMessage(false, "Safety Loop", "System operational");
    xr4_system_context.system_state = XR4_STATE_ARMED;
    
    // 4. Handle headlights (non-critical)
    xr4_system_context.Headlights_On = SN_Switches_AreHeadlightsOn();
}

// ========================================
// Example 8: Interrupt Response Time Test
// ========================================
void example_interrupt_timing_test() {
    static unsigned long lastEStopTime = 0;
    
    SwitchStates_t states = SN_Switches_GetStates();
    
    if (states.estop_triggered) {
        unsigned long responseTime = millis() - lastEStopTime;
        logMessage(true, "Timing", "E-STOP response time: %lu ms", responseTime);
        lastEStopTime = millis();
    }
}

// ========================================
// Example 9: Encoder Position Tracking
// ========================================
void example_encoder_position_tracking() {
    int16_t position = SN_Switches_GetEncoderPosition();
    
    logMessage(false, "Encoder", "Absolute position: %d", position);
    
    // Reset encoder if it reaches a limit
    if (abs(position) > 1000) {
        SN_Switches_ResetEncoder();
        logMessage(true, "Encoder", "Position reset to 0");
    }
}

// ========================================
// Example 10: Integration with LCD Display
// ========================================
void example_lcd_integration() {
    static char lcdBuffer[21]; // 20 chars + null
    
    // Display switch states on LCD
    bool armed = SN_Switches_IsArmed();
    bool estop = SN_Switches_IsEStopActive();
    bool lights = SN_Switches_AreHeadlightsOn();
    int16_t encoder = SN_Switches_GetEncoderPosition();
    
    // Format status line
    snprintf(lcdBuffer, sizeof(lcdBuffer), "A:%c E:%c L:%c E:%d",
             armed ? 'Y' : 'N',
             estop ? '!' : '-',
             lights ? 'Y' : 'N',
             encoder);
    
    logMessage(false, "LCD", "Status: %s", lcdBuffer);
    // Update LCD with lcdBuffer
}

// ========================================
// Main Loop Integration Example
// ========================================
void example_main_loop() {
    // This is how to integrate into the main loop
    
    // Update switches (called every loop iteration)
    SN_Switches_Update();
    
    // Perform safety checks
    example_complete_safety_loop();
    
    // Handle switch changes
    SwitchStates_t states = SN_Switches_GetStates();
    
    if (states.arm_changed) {
        example_arming_logic();
    }
    
    if (states.headlights_changed) {
        example_headlights_control();
    }
    
    if (states.estop_triggered) {
        // E-STOP is handled immediately by interrupt
        // But you can add additional logging/notification here
        logMessage(true, "Main Loop", "E-STOP event detected in main loop");
    }
    
    // Handle encoder for UI
    if (states.encoder_delta != 0 || states.rotary_pressed) {
        example_encoder_menu_navigation();
    }
}

#endif // SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

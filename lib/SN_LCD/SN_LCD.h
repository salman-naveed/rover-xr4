#ifndef SN_LCD_H
#define SN_LCD_H

#include <SN_Common.h>  // For xr4_system_context_t

// ========================================
// LCD Page Types
// ========================================
enum LCD_Page {
    LCD_PAGE_STATUS = 0,      // System status, state machine, connection
    LCD_PAGE_TELEMETRY,       // Voltage, current, temperature
    LCD_PAGE_GPS,             // GPS position, fix status
    LCD_PAGE_SENSORS,         // Gyro, accelerometer, magnetometer
    LCD_PAGE_CONTROL,         // Joystick, encoder, switches
    LCD_PAGE_COUNT            // Total number of pages
};

// ========================================
// LCD State Structure
// ========================================
typedef struct {
    LCD_Page current_page;
    bool needs_redraw;
    bool is_initialized;
    unsigned long last_update_time;
    unsigned long last_blink_time;
    bool blink_state;
    uint8_t scroll_offset;
} LCD_State_t;

// ========================================
// Core LCD Functions (Backward Compatible)
// ========================================
void SN_LCD_Init();
void SN_LCD_Clear();
void SN_LCD_Print(String message);
void SN_LCD_PrintAt(int col, int row, String message);
void SN_LCD_PrintAt(int col, int row, int message);
void SN_LCD_PrintAt(int col, int row, float message);
void SN_LCD_PrintAt(int col, int row, double message);

// ========================================
// Multi-Page LCD Functions (Non-Blocking)
// ========================================

/**
 * Update LCD display - MUST be called regularly from main loop
 * Non-blocking: Updates at configurable intervals (default 100ms)
 * Returns immediately if not time to update yet
 */
void SN_LCD_Update(xr4_system_context_t* system_context);

/**
 * Navigate to next page (called when encoder rotates CW)
 */
void SN_LCD_NextPage();

/**
 * Navigate to previous page (called when encoder rotates CCW)
 */
void SN_LCD_PrevPage();

/**
 * Get current page number
 */
LCD_Page SN_LCD_GetCurrentPage();

/**
 * Force immediate redraw of current page
 */
void SN_LCD_ForceRedraw();

/**
 * Check if LCD is initialized and ready
 */
bool SN_LCD_IsReady();

/**
 * Set update interval in milliseconds (default: 100ms)
 * Faster updates = more CPU usage, slower = less responsive
 * Recommended: 50-200ms for good balance
 */
void SN_LCD_SetUpdateInterval(unsigned long interval_ms);
#endif // SN_LCD_H

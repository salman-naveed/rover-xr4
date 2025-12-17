// ========================================
// SN_LCD.cpp - Multi-Page LCD Display Implementation
// ========================================

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <SN_LCD.h>
#include <SN_Logger.h>
#include <SN_Common.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

// ========================================
// LCD Hardware Configuration
// ========================================
#define LCD_ADDRESS 0x27
#define LCD_ROWS 4
#define LCD_COLUMNS 20
#define LCD_BACKLIGHT 255

// ========================================
// LCD Update Timing Configuration
// ========================================
#define LCD_DEFAULT_UPDATE_INTERVAL_MS 100    // Update every 100ms (10Hz)
#define LCD_BLINK_INTERVAL_MS 500             // Blink rate for indicators

// ========================================
// External Dependencies
// ========================================
extern bool espnow_init_success; // Flag to check if ESP-NOW is initialized

// ========================================
// Custom Characters
// ========================================
byte lcd_startup_char[8] =  { 
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

byte lcd_arrow_up[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00000
};

byte lcd_arrow_down[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000
};

byte lcd_degree[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000
};

// ========================================
// Global Variables
// ========================================
LiquidCrystal_PCF8574 lcd(LCD_ADDRESS);
LCD_State_t lcd_state;
unsigned long lcd_update_interval = LCD_DEFAULT_UPDATE_INTERVAL_MS;

// ========================================
// Helper Functions (Internal)
// ========================================

/**
 * Convert system state enum to human-readable string (max 8 chars for display)
 */
const char* getStateString(uint8_t state) {
    switch(state) {
        case XR4_STATE_JUST_POWERED_ON: return "POWERUP";
        case XR4_STATE_INITIALIZED:     return "INIT";
        case XR4_STATE_COMMS_CONFIG:    return "COMMS";
        case XR4_STATE_WAITING_FOR_ARM: return "WAITING";
        case XR4_STATE_ARMED:           return "ARMED";
        case XR4_STATE_ERROR:           return "ERROR";
        case XR4_STATE_EMERGENCY_STOP:  return "E-STOP";
        case XR4_STATE_REBOOT:          return "REBOOT";
        default:                        return "UNKNOWN";
    }
}

/**
 * Print centered text on LCD
 */
void printCentered(int row, const char* text) {
    int len = strlen(text);
    int col = (LCD_COLUMNS - len) / 2;
    if (col < 0) col = 0;
    lcd.setCursor(col, row);
    lcd.print(text);
}

/**
 * Print right-aligned text on LCD
 */
void printRightAlign(int row, const char* text) {
    int len = strlen(text);
    int col = LCD_COLUMNS - len;
    if (col < 0) col = 0;
    lcd.setCursor(col, row);
    lcd.print(text);
}

/**
 * Format voltage for display (e.g., "12.34V")
 */
void printVoltage(int col, int row, float voltage) {
    lcd.setCursor(col, row);
    lcd.print(voltage, 2);
    lcd.print("V");
}

/**
 * Format current for display (e.g., "1.23A")
 */
void printCurrent(int col, int row, float current) {
    lcd.setCursor(col, row);
    lcd.print(current, 2);
    lcd.print("A");
}

/**
 * Format temperature for display (e.g., "25.3C")
 */
void printTemperature(int col, int row, float temp) {
    lcd.setCursor(col, row);
    lcd.print(temp, 1);
    lcd.write(3); // Degree symbol (custom char)
    lcd.print("C");
}

/**
 * Format GPS coordinate for display (e.g., "12.345678")
 */
void printGPSCoord(int col, int row, double coord) {
    lcd.setCursor(col, row);
    if (coord == 0.0) {
        lcd.print("---.------");
    } else {
        lcd.print(coord, 6);
    }
}

// ========================================
// Page Rendering Functions
// ========================================

/**
 * Render STATUS page (Page 0)
 * Shows: System state, ARM status, E-STOP, ESP-NOW connection
 */
void renderStatusPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    lcd.setCursor(0, 0);
    lcd.print("=== ROVER XR-4 === ");  // 20 chars total
    
    // Row 1: State and ARM status  
    lcd.setCursor(0, 1);
    lcd.print("State: ");
    lcd.print(getStateString(ctx->system_state));
    lcd.print("           ");  // Clear remaining chars
    
    // Row 2: E-STOP and ARM indicators
    lcd.setCursor(0, 2);
    if (ctx->Emergency_Stop) {
        lcd.print("E-STOP! ");
    } else {
        lcd.print("NORMAL  ");
    }
    
    lcd.print(ctx->Armed ? "ARMED  " : "DISARM ");
    lcd.print("    ");  // Clear remaining
    
    // Row 3: ESP-NOW connection status (blinking indicator)
    lcd.setCursor(0, 3);
    lcd.print("Link:");
    if (espnow_init_success) {
        if (lcd_state.blink_state) {
            lcd.print("OK ");
        } else {
            lcd.print("ok ");
        }
        // Show RSSI if available
        lcd.print("OBC:");
        if (ctx->OBC_RSSI < 0) {
            lcd.print((int)ctx->OBC_RSSI);
        } else {
            lcd.print("--");
        }
        lcd.print("dB  ");  // Padding
    } else {
        lcd.print("NO CONN      ");  // 14 chars total after "Link:"
    }
}

/**
 * Render TELEMETRY page (Page 1)
 * Shows: Voltage, Current, Temperature, Power
 */
void renderTelemetryPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    printCentered(0, "TELEMETRY");
    
    // Row 1: Voltage
    lcd.setCursor(0, 1);
    lcd.print("Volt: ");
    printVoltage(6, 1, ctx->Main_Bus_V);
    
    // Row 2: Current
    lcd.setCursor(0, 2);
    lcd.print("Curr: ");
    printCurrent(6, 2, ctx->Main_Bus_I);
    
    // Row 3: Temperature and Power
    lcd.setCursor(0, 3);
    lcd.print("Temp:");
    printTemperature(5, 3, ctx->temp);
    
    lcd.setCursor(11, 3);
    float power = ctx->Main_Bus_V * ctx->Main_Bus_I;
    lcd.print(power, 1);
    lcd.print("W");
}

/**
 * Render GPS page (Page 2)
 * Shows: GPS fix status, latitude, longitude
 */
void renderGPSPage(xr4_system_context_t* ctx) {
    // Row 0: Title and fix status
    lcd.setCursor(0, 0);
    lcd.print("GPS: ");
    if (ctx->GPS_fix) {
        lcd.print("FIX  ");
    } else {
        lcd.print("NO FIX");
    }
    
    // Row 1: Latitude
    lcd.setCursor(0, 1);
    lcd.print("Lat: ");
    printGPSCoord(5, 1, ctx->GPS_lat);
    
    // Row 2: Longitude
    lcd.setCursor(0, 2);
    lcd.print("Lon: ");
    printGPSCoord(5, 2, ctx->GPS_lon);
    
    // Row 3: Time (if available)
    lcd.setCursor(0, 3);
    lcd.print("Time: ");
    if (ctx->GPS_time > 0) {
        lcd.print(ctx->GPS_time, 2);
    } else {
        lcd.print("--:--:--");
    }
}

/**
 * Render SENSORS page (Page 3)
 * Shows: Gyro, Accelerometer, Magnetometer
 */
void renderSensorsPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    printCentered(0, "SENSORS");
    
    // Row 1: Gyroscope
    lcd.setCursor(0, 1);
    lcd.print("G:");
    lcd.print(ctx->Gyro_X, 1);
    lcd.print(" ");
    lcd.print(ctx->Gyro_Y, 1);
    lcd.print(" ");
    lcd.print(ctx->Gyro_Z, 1);
    
    // Row 2: Accelerometer
    lcd.setCursor(0, 2);
    lcd.print("A:");
    lcd.print(ctx->Acc_X, 1);
    lcd.print(" ");
    lcd.print(ctx->Acc_Y, 1);
    lcd.print(" ");
    lcd.print(ctx->Acc_Z, 1);
    
    // Row 3: Magnetometer
    lcd.setCursor(0, 3);
    lcd.print("M:");
    lcd.print(ctx->Mag_X, 0);
    lcd.print(" ");
    lcd.print(ctx->Mag_Y, 0);
    lcd.print(" ");
    lcd.print(ctx->Mag_Z, 0);
}

/**
 * Render CONTROL page (Page 4)
 * Shows: Joystick position, Encoder, Switches
 */
void renderControlPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    printCentered(0, "CONTROL INPUTS");
    
    // Row 1: Joystick X & Y
    lcd.setCursor(0, 1);
    lcd.print("Joy X:");
    lcd.print(ctx->Joystick_X);
    lcd.print("  Y:");
    lcd.print(ctx->Joystick_Y);
    lcd.print("  ");
    
    // Row 2: Encoder position
    lcd.setCursor(0, 2);
    lcd.print("Encoder: ");
    lcd.print(ctx->Encoder_Pos);
    lcd.print("    ");
    
    // Row 3: Switches status
    lcd.setCursor(0, 3);
    lcd.print("H:");
    lcd.print(ctx->Headlights_On ? "ON " : "OFF");
    lcd.print(" A:");
    lcd.print(ctx->Armed ? "Y" : "N");
    lcd.print(" E:");
    lcd.print(ctx->Emergency_Stop ? "Y" : "N");
}

// ========================================
// Public API Implementation
// ========================================

void SN_LCD_Init(){
    Wire.begin();
    Wire.available();
    Wire.beginTransmission(LCD_ADDRESS);
    uint8_t error = Wire.endTransmission();

    logMessage(true, "SN_LCD_Init", "Scanning for LCD at address 0x%02X", LCD_ADDRESS);

    if (error == 0) {
        logMessage(true, "SN_LCD_Init", "LCD found at address 0x%02X", LCD_ADDRESS);
        
        // Initialize LCD hardware
        lcd.begin(LCD_COLUMNS, LCD_ROWS);
        lcd.setBacklight(LCD_BACKLIGHT);
        
        // Create custom characters
        lcd.createChar(1, lcd_startup_char);
        lcd.createChar(2, lcd_arrow_up);
        lcd.createChar(3, lcd_degree);
        
        // Initialize LCD state
        lcd_state.current_page = LCD_PAGE_STATUS;
        lcd_state.needs_redraw = true;
        lcd_state.is_initialized = true;
        lcd_state.last_update_time = 0;
        lcd_state.last_blink_time = 0;
        lcd_state.blink_state = false;
        lcd_state.scroll_offset = 0;
        
        // Show startup splash
        SN_LCD_Clear();
        printCentered(1, "ROVER XR-4");
        printCentered(2, "Initializing...");
        
        delay(1500); // Brief startup display
        
        // Clear and prepare for first page render
        SN_LCD_Clear();
        SN_LCD_ForceRedraw(); // Trigger immediate redraw to first page
        
    } else {
        logMessage(true, "SN_LCD_Init", "LCD not found at address 0x%02X, Error: %d", LCD_ADDRESS, error);
        lcd_state.is_initialized = false;
    }
}

void SN_LCD_Update(xr4_system_context_t* system_context) {
    if (!lcd_state.is_initialized) {
        return; // LCD not available, skip update
    }
    
    unsigned long current_time = millis();
    
    // Update blink state for indicators
    if (current_time - lcd_state.last_blink_time >= LCD_BLINK_INTERVAL_MS) {
        lcd_state.blink_state = !lcd_state.blink_state;
        lcd_state.last_blink_time = current_time;
        // Don't set needs_redraw - blink handled in render functions
    }
    
    // Non-blocking update: only update at specified interval
    if (!lcd_state.needs_redraw && 
        (current_time - lcd_state.last_update_time < lcd_update_interval)) {
        return; // Not time to update yet
    }
    
    // Only clear LCD when changing pages to prevent flickering
    // For regular updates, just overwrite the content
    static LCD_Page last_rendered_page = LCD_PAGE_STATUS;
    if (lcd_state.current_page != last_rendered_page || lcd_state.needs_redraw) {
        lcd.clear();
        last_rendered_page = lcd_state.current_page;
    }
    
    switch (lcd_state.current_page) {
        case LCD_PAGE_STATUS:
            renderStatusPage(system_context);
            break;
        case LCD_PAGE_TELEMETRY:
            renderTelemetryPage(system_context);
            break;
        case LCD_PAGE_GPS:
            renderGPSPage(system_context);
            break;
        case LCD_PAGE_SENSORS:
            renderSensorsPage(system_context);
            break;
        case LCD_PAGE_CONTROL:
            renderControlPage(system_context);
            break;
        default:
            lcd_state.current_page = LCD_PAGE_STATUS;
            renderStatusPage(system_context);
            break;
    }
    
    // Show page indicator on all pages (bottom right corner)
    // Position: "X/5" where X is current page (1-5)
    // For 20x4 display: Column 17-19 (0-indexed), Row 3
    lcd.setCursor(17, 3);
    lcd.print(lcd_state.current_page + 1);
    lcd.print("/");
    lcd.print(LCD_PAGE_COUNT);
    lcd.print(" ");  // Clear any trailing characters
    
    lcd_state.last_update_time = current_time;
    lcd_state.needs_redraw = false;
}

void SN_LCD_NextPage() {
    if (!lcd_state.is_initialized) return;
    
    lcd_state.current_page = (LCD_Page)((lcd_state.current_page + 1) % LCD_PAGE_COUNT);
    lcd_state.needs_redraw = true;
    
    logMessage(true, "SN_LCD", "Navigated to page %d", lcd_state.current_page);
}

void SN_LCD_PrevPage() {
    if (!lcd_state.is_initialized) return;
    
    if (lcd_state.current_page == 0) {
        lcd_state.current_page = (LCD_Page)(LCD_PAGE_COUNT - 1);
    } else {
        lcd_state.current_page = (LCD_Page)(lcd_state.current_page - 1);
    }
    lcd_state.needs_redraw = true;
    
    logMessage(true, "SN_LCD", "Navigated to page %d", lcd_state.current_page);
}

LCD_Page SN_LCD_GetCurrentPage() {
    return lcd_state.current_page;
}

void SN_LCD_ForceRedraw() {
    lcd_state.needs_redraw = true;
}

bool SN_LCD_IsReady() {
    return lcd_state.is_initialized;
}

void SN_LCD_SetUpdateInterval(unsigned long interval_ms) {
    lcd_update_interval = interval_ms;
    logMessage(true, "SN_LCD", "Update interval set to %lu ms", interval_ms);
}

// ========================================
// Backward Compatible Functions
// ========================================

void SN_LCD_Clear(){
    if (lcd_state.is_initialized) {
        lcd.clear();
    }
}

void SN_LCD_Print(String message){
    if (lcd_state.is_initialized) {
        lcd.clear();
        lcd.print(message);
    }
}

void SN_LCD_PrintAt(int col, int row, String message){
    if (lcd_state.is_initialized) {
        lcd.setCursor(col, row);
        lcd.print(message);
    }
}

void SN_LCD_PrintAt(int col, int row, int message){
    if (lcd_state.is_initialized) {
        lcd.setCursor(col, row);
        lcd.print(message);
    }
}

void SN_LCD_PrintAt(int col, int row, float message){
    if (lcd_state.is_initialized) {
        lcd.setCursor(col, row);
        lcd.print(message);
    }
}

void SN_LCD_PrintAt(int col, int row, double message){
    if (lcd_state.is_initialized) {
        lcd.setCursor(col, row);
        lcd.print(message);
    }
}

#endif // SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
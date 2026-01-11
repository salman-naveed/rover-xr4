// ========================================
// SN_LCD.cpp - Multi-Page LCD Display Implementation
// ========================================

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <SN_LCD.h>
#include <SN_Logger.h>
#include <SN_Common.h>
#include <SN_ESPNOW.h>  // For connection status check

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
#define LCD_DEFAULT_UPDATE_INTERVAL_MS 200    // Update every 200ms (5Hz) - optimized for reduced I2C congestion
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
    
    // Print with padding to clear old digits (format: "XX.XXA  " - 7 chars total)
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%5.2fA ", current);
    lcd.print(buffer);
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
 * Shows: System state, ARM status, E-STOP, ESP-NOW connection, Battery warning
 */
void renderStatusPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    lcd.setCursor(0, 0);
    lcd.print(" === ROVER XR-4 === ");  // 20 chars total
    
    // Row 1: State and Battery Warning
    lcd.setCursor(0, 1);
    
    // Check battery voltage and show warning if critical/low
    // For 3S LiPo: Critical < 9.0V (3.0V/cell), Warning < 10.5V (3.5V/cell)
    if (ctx->Main_Bus_V > 0.1 && ctx->Main_Bus_V < 9.0) {
        // Critical battery voltage
        lcd.print("BAT CRITICAL! ");
        if (lcd_state.blink_state) {
            lcd.print(ctx->Main_Bus_V, 1);
            lcd.print("V");
        } else {
            lcd.print("     ");  // Blink the voltage
        }
    } else if (ctx->Main_Bus_V > 0.1 && ctx->Main_Bus_V < 10.5) {
        // Low battery voltage
        lcd.print("BAT LOW! ");
        lcd.print(ctx->Main_Bus_V, 1);
        lcd.print("V    ");
    } else {
        // Normal - show state
        lcd.print("State: ");
        lcd.print(getStateString(ctx->system_state));
        lcd.print("           ");  // Clear remaining chars
    }
    
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
        // Check if we're actually receiving data (not just initialized)
        bool connected = SN_ESPNOW_IsConnected();
        
        if (connected) {
            // Connected - show blinking OK
            if (lcd_state.blink_state) {
                lcd.print("OK ");
            } else {
                lcd.print("ok ");
            }
            // Show RSSI if available
            lcd.print("OBC:");
            if (ctx->CTU_RSSI < 0) {
                lcd.print((int)ctx->CTU_RSSI);
            } else {
                lcd.print("--");
            }
            lcd.print("dB  ");  // Padding
        } else {
            // Init OK but no data received
            lcd.print("NO DATA!    ");
        }
    } else {
        lcd.print("NO CONN      ");  // 14 chars total after "Link:"
    }
}

/**
 * Render TELEMETRY page (Page 1)
 * Shows: Main/5V/3.3V voltage, current, and power consumption
 * 
 * Layout (20x4):
 * Row 0: "    TELEMETRY      " (centered title)
 * Row 1: "12.30V  5.10V 3.31V" (Voltages: Main, 5V, 3.3V)
 * Row 2: " 1.00A  0.50A 0.20A" (Currents: Main, 5V, 3.3V)
 * Row 3: "12.30W  2.55W 0.66W" (Power: Main, 5V, 3.3V)
 */
void renderTelemetryPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    printCentered(0, "TELEMETRY");
    
    // Row 1: All Voltages (Main, 5V, 3.3V)
    lcd.setCursor(0, 1);
    // Main voltage (6 chars: "XX.XXV")
    char vmain[7];
    snprintf(vmain, sizeof(vmain), "%5.2fV", ctx->Main_Bus_V);
    lcd.print(vmain);
    lcd.print(" ");
    // 5V voltage (6 chars: "X.XXV")
    char v5[6];
    snprintf(v5, sizeof(v5), "%4.2fV", ctx->Bus_5V);
    lcd.print(v5);
    lcd.print(" ");
    // 3.3V voltage (5 chars: "X.XXV")
    char v3[6];
    snprintf(v3, sizeof(v3), "%4.2fV", ctx->Bus_3V3);
    lcd.print(v3);
    
    // Row 2: All Currents (Main, 5V, 3.3V)
    lcd.setCursor(0, 2);
    // Main current (6 chars: "X.XXA")
    char imain[7];
    snprintf(imain, sizeof(imain), "%5.2fA", ctx->Main_Bus_I);
    lcd.print(imain);
    lcd.print(" ");
    // 5V current (6 chars: "X.XXA") - placeholder, sensor not installed
    lcd.print(" -.--A");
    lcd.print(" ");
    // 3.3V current (5 chars: "X.XXA") - placeholder, sensor not installed
    lcd.print("-.--A");
    
    // Row 3: All Power (Main, 5V, 3.3V)
    lcd.setCursor(0, 3);
    // Main power
    float power_main = ctx->Main_Bus_V * ctx->Main_Bus_I;
    char pmain[7];
    snprintf(pmain, sizeof(pmain), "%5.2fW", power_main);
    lcd.print(pmain);
    lcd.print(" ");
    // 5V power - placeholder
    lcd.print(" -.--W");
    lcd.print(" ");
    // 3.3V power - placeholder
    lcd.print("-.--W");
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
 * Render IMU page (Page 3)
 * Shows: Orientation (Heading, Pitch, Roll)
 */
void renderSensorsPage(xr4_system_context_t* ctx) {
    // Row 0: Title
    printCentered(0, "IMU");
    
    // Row 1: Heading with cardinal direction
    lcd.setCursor(0, 1);
    lcd.print("Hdg: ");
    lcd.print((int)ctx->Heading_Degrees);
    lcd.write(0xDF);  // Degree symbol
    lcd.print(" ");
    lcd.print(ctx->Heading_Cardinal);
    lcd.print("     ");  // Clear remainder
    
    // Row 2: Pitch angle
    lcd.setCursor(0, 2);
    lcd.print("Pitch: ");
    if (ctx->Pitch_Degrees >= 0) lcd.print("+");
    lcd.print(ctx->Pitch_Degrees, 1);
    lcd.write(0xDF);  // Degree symbol
    lcd.print("    ");  // Clear remainder
    
    // Row 3: Roll angle
    lcd.setCursor(0, 3);
    lcd.print("Roll:  ");
    if (ctx->Roll_Degrees >= 0) lcd.print("+");
    lcd.print(ctx->Roll_Degrees, 1);
    lcd.write(0xDF);  // Degree symbol
    lcd.print("    ");  // Clear remainder
}

/**
 * Render CONTROL page (Page 4)
 * Shows: Joystick X/Y, Headlights, Switch position, Temperature
 * 
 * Layout (20x4):
 * Row 0: "JoyX:1856 JoyY:1880" (Joystick raw values)
 * Row 1: "Hdlt:OFF  Sw:A     " (Headlights + Switch state)
 * Row 2: "Temp:25.3°C         " (Temperature)
 * Row 3: "                    " (Reserved/empty)
 */
void renderControlPage(xr4_system_context_t* ctx) {
    // Row 0: Joystick X & Y (no title, more space)
    lcd.setCursor(0, 0);
    lcd.print("JoyX:");
    lcd.print(ctx->Joystick_X);
    lcd.print(" JoyY:");
    lcd.print(ctx->Joystick_Y);
    lcd.print("     ");  // Clear remainder
    
    // Row 1: Headlights status
    lcd.setCursor(0, 1);
    lcd.print("Hdlt:");
    lcd.print(ctx->Headlights_On ? "ON " : "OFF");
    
    // Row 1 (continued): Three-position switch status (Button A/B/C)
    lcd.print("  Sw:");
    if (ctx->Button_A) {
        lcd.print("A");
    } else if (ctx->Button_B) {
        lcd.print("B");
    } else if (ctx->Button_C) {
        lcd.print("C");
    } else {
        lcd.print("-");  // None pressed
    }
    lcd.print("     ");  // Clear remainder
    
    // Row 2: Temperature
    lcd.setCursor(0, 2);
    lcd.print("Temp:");
    printTemperature(5, 2, ctx->temp);
    lcd.print("          ");  // Clear remainder
    
    // Row 3: Reserved/empty
    lcd.setCursor(0, 3);
    lcd.print("                    ");  // Clear entire row
}

void renderDiagnosticsPage() {
    // Row 0: Title
    lcd.setCursor(0, 0);
    lcd.print("  << DIAGNOSTICS >> ");
    
    // Row 1: Telemetry packets received
    lcd.setCursor(0, 1);
    lcd.print("RX: ");
    lcd.print(SN_ESPNOW_GetTelemetryPacketsReceived());
    lcd.print("     ");  // Clear remainder
    
    // Row 2: Telecommand packets sent and failures
    lcd.setCursor(0, 2);
    lcd.print("TX: ");
    lcd.print(SN_ESPNOW_GetTelecommandPacketsSent());
    lcd.print(" Fail: ");
    lcd.print(SN_ESPNOW_GetTelecommandSendFailures());
    lcd.print("     ");  // Clear remainder
    
    // Row 3: Uptime in minutes
    lcd.setCursor(0, 3);
    lcd.print("Uptime: ");
    unsigned long uptime_minutes = millis() / 60000;
    lcd.print(uptime_minutes);
    lcd.print(" min");
    lcd.print("          ");  // Clear remainder
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
        
        // Show startup splash with firmware version
        SN_LCD_Clear();
        printCentered(1, "ROVER XR-4");
        printCentered(2, FIRMWARE_VERSION_STRING);
        
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
        case LCD_PAGE_DIAGNOSTICS:
            renderDiagnosticsPage();
            break;
        default:
            lcd_state.current_page = LCD_PAGE_STATUS;
            renderStatusPage(system_context);
            break;
    }
    
    // Show page indicator on all pages (bottom right corner)
    // Position: "X/5" where X is current page (1-5)
    // For 20x4 display: Column 17-19 (0-indexed), Row 3
    // lcd.setCursor(17, 3);
    // lcd.print(lcd_state.current_page + 1);
    // lcd.print("/");
    // lcd.print(LCD_PAGE_COUNT);
    // lcd.print(" ");  // Clear any trailing characters
    
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
/*
 * OBC Simulator Test Sketch for ESP32
 * 
 * This sketch simulates an OBC (On-Board Computer) that:
 * 1. Receives telecommands from the CTU via ESP-NOW
 * 2. Displays received telecommand data on Serial Monitor
 * 3. Sends telemetry back to CTU with placeholder values
 * 
 * Hardware Required:
 * - ESP32 board (any variant)
 * - USB cable for serial monitoring
 * 
 * Configuration:
 * - Set the CTU MAC address below to match your actual CTU board
 * - Upload this sketch to a separate ESP32 (not the CTU)
 * - Open Serial Monitor at 115200 baud
 * 
 * Compatible with ESP32 Arduino Core 2.x and 3.x
 * 
 * Author: GitHub Copilot
 * Date: December 15, 2025
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// Check ESP32 Arduino Core version for callback compatibility
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  #define ESP_NOW_CORE_3X
#endif

// ========================================
// Configuration
// ========================================
#define SERIAL_BAUD_RATE 115200
#define TELEMETRY_SEND_INTERVAL_MS 500  // Send telemetry every 500ms

// *** IMPORTANT: Set this to your CTU's MAC address ***
// You can find your CTU's MAC address by running WiFi.macAddress() on the CTU
// Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
uint8_t ctuMacAddress[] = {0x24, 0x0a, 0xc4, 0xc0, 0xf1, 0xec};  // CHANGE THIS!

// CTU MAC: 24:0a:c4:c0:f1:ec

// ========================================
// Data Structures (from SN_ESPNOW.h)
// ========================================
typedef enum {
    TM_GPS_DATA_MSG = 0x10,     // GPS data
    TM_IMU_DATA_MSG = 0x20,     // IMU data
    TM_HK_DATA_MSG = 0x30,      // Housekeeping data
} telemetry_message_type_id_t;

typedef enum {
    TC_C2_DATA_MSG = 0x11,      // Control & Commands data
} telecommand_message_type_id_t;

// Telemetry structures (OBC --> CTU)
typedef struct telemetry_GPS_data {
    uint8_t msg_type = TM_GPS_DATA_MSG;
    double GPS_lat;
    double GPS_lon;
    double GPS_time;
    bool GPS_fix;
} telemetry_GPS_data_t;

typedef struct telemetry_IMU_data {
    uint8_t msg_type = TM_IMU_DATA_MSG;
    float Gyro_X;
    float Gyro_Y;
    float Gyro_Z;
    float Acc_X;
    float Acc_Y;
    float Acc_Z;
    float Mag_X;
    float Mag_Y;
    float Mag_Z;
} telemetry_IMU_data_t;

typedef struct telemetry_HK_data {
    uint8_t msg_type = TM_HK_DATA_MSG;
    float Main_Bus_V;
    float Main_Bus_I;
    float temp;
    float OBC_RSSI;
} telemetry_HK_data_t;

// Telecommand structure (CTU --> OBC)
typedef struct telecommand_data {
    uint8_t msg_type = TC_C2_DATA_MSG;
    uint16_t Command;
    uint16_t Joystick_X;
    uint16_t Joystick_Y;
    uint16_t Encoder_Pos;
    uint16_t flags;         // | Emergency_Stop | Armed | Button_A | Button_B | Button_C | Button_D | Headlights_On | Buzzer |
    uint16_t CTU_RSSI;
} telecommand_data_t;

// ========================================
// Global Variables
// ========================================
telecommand_data_t received_telecommand;
telemetry_GPS_data_t gps_telemetry;
telemetry_IMU_data_t imu_telemetry;
telemetry_HK_data_t hk_telemetry;

unsigned long last_telemetry_send = 0;
unsigned long last_telecommand_received = 0;
uint8_t telemetry_cycle = 0;  // Cycle between GPS, IMU, HK

bool telecommand_received = false;
int telecommand_count = 0;
int telemetry_sent_count = 0;

esp_now_peer_info_t peerInfo;

// ========================================
// Helper Functions
// ========================================

// Extract flag bits from telecommand
bool getFlag(uint16_t flags, uint8_t bit_position) {
    return (flags & (1 << bit_position)) != 0;
}

void printTelecommandData() {
    Serial.println("\n========== TELECOMMAND RECEIVED ==========");
    Serial.printf("Message Type: 0x%02X\n", received_telecommand.msg_type);
    Serial.printf("Command: 0x%04X\n", received_telecommand.Command);
    Serial.printf("Joystick X: %d (Raw ADC)\n", received_telecommand.Joystick_X);
    Serial.printf("Joystick Y: %d (Raw ADC)\n", received_telecommand.Joystick_Y);
    Serial.printf("Encoder Position: %d\n", received_telecommand.Encoder_Pos);
    Serial.printf("CTU RSSI: %d dBm\n", received_telecommand.CTU_RSSI);
    
    Serial.println("\nControl Flags:");
    Serial.printf("  Emergency Stop: %s\n", getFlag(received_telecommand.flags, 0) ? "ACTIVE" : "Inactive");
    Serial.printf("  Armed: %s\n", getFlag(received_telecommand.flags, 1) ? "YES" : "NO");
    Serial.printf("  Button A: %s\n", getFlag(received_telecommand.flags, 2) ? "Pressed" : "Released");
    Serial.printf("  Button B: %s\n", getFlag(received_telecommand.flags, 3) ? "Pressed" : "Released");
    Serial.printf("  Button C: %s\n", getFlag(received_telecommand.flags, 4) ? "Pressed" : "Released");
    Serial.printf("  Button D: %s\n", getFlag(received_telecommand.flags, 5) ? "Pressed" : "Released");
    Serial.printf("  Headlights: %s\n", getFlag(received_telecommand.flags, 6) ? "ON" : "OFF");
    Serial.printf("  Buzzer: %s\n", getFlag(received_telecommand.flags, 7) ? "ON" : "OFF");
    Serial.println("==========================================\n");
}

void updatePlaceholderTelemetry() {
    unsigned long currentTime = millis();
    float timeInSeconds = currentTime / 1000.0;
    
    // GPS Telemetry (simulated values)
    gps_telemetry.GPS_lat = 37.7749 + sin(timeInSeconds * 0.1) * 0.001;  // Simulated latitude near San Francisco
    gps_telemetry.GPS_lon = -122.4194 + cos(timeInSeconds * 0.1) * 0.001; // Simulated longitude
    gps_telemetry.GPS_time = timeInSeconds;
    gps_telemetry.GPS_fix = true;
    
    // IMU Telemetry (simulated sensor readings)
    imu_telemetry.Gyro_X = sin(timeInSeconds) * 10.0;
    imu_telemetry.Gyro_Y = cos(timeInSeconds) * 10.0;
    imu_telemetry.Gyro_Z = sin(timeInSeconds * 0.5) * 5.0;
    imu_telemetry.Acc_X = sin(timeInSeconds * 0.3) * 2.0;
    imu_telemetry.Acc_Y = cos(timeInSeconds * 0.3) * 2.0;
    imu_telemetry.Acc_Z = 9.81 + sin(timeInSeconds * 0.2) * 0.5;  // Gravity + noise
    imu_telemetry.Mag_X = 25.0 + sin(timeInSeconds * 0.1) * 5.0;
    imu_telemetry.Mag_Y = 30.0 + cos(timeInSeconds * 0.1) * 5.0;
    imu_telemetry.Mag_Z = 35.0 + sin(timeInSeconds * 0.15) * 3.0;
    
    // Housekeeping Telemetry (simulated battery/temperature)
    hk_telemetry.Main_Bus_V = 12.0 + sin(timeInSeconds * 0.05) * 0.5;  // 11.5-12.5V
    hk_telemetry.Main_Bus_I = 2.5 + cos(timeInSeconds * 0.08) * 0.3;   // 2.2-2.8A
    hk_telemetry.temp = 25.0 + sin(timeInSeconds * 0.02) * 5.0;        // 20-30°C
    hk_telemetry.OBC_RSSI = -45.0 + random(-10, 10);                   // Simulated RSSI
}

// ========================================
// ESP-NOW Callbacks
// ========================================

// Callback when data is received from CTU
// Signature differs between ESP32 Arduino Core 2.x and 3.x
#ifdef ESP_NOW_CORE_3X
// ESP32 Core 3.x callback (uses esp_now_recv_info_t)
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
    telecommand_count++;
    last_telecommand_received = millis();
    
    // Check if it's a telecommand message
    if (len == sizeof(telecommand_data_t)) {
        memcpy(&received_telecommand, incomingData, sizeof(received_telecommand));
        
        if (received_telecommand.msg_type == TC_C2_DATA_MSG) {
            telecommand_received = true;
            printTelecommandData();
        }
    } else {
        Serial.printf("Received unknown message (size: %d bytes)\n", len);
    }
}
#else
// ESP32 Core 2.x callback (uses MAC address directly)
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    telecommand_count++;
    last_telecommand_received = millis();
    
    // Check if it's a telecommand message
    if (len == sizeof(telecommand_data_t)) {
        memcpy(&received_telecommand, incomingData, sizeof(received_telecommand));
        
        if (received_telecommand.msg_type == TC_C2_DATA_MSG) {
            telecommand_received = true;
            printTelecommandData();
        }
    } else {
        Serial.printf("Received unknown message (size: %d bytes)\n", len);
    }
}
#endif

// Callback when data is sent to CTU
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.print("✓ Telemetry sent successfully");
    } else {
        Serial.print("✗ Telemetry send failed");
    }
    
    // Print which telemetry type was sent
    switch (telemetry_cycle) {
        case 0:
            Serial.println(" (GPS)");
            break;
        case 1:
            Serial.println(" (IMU)");
            break;
        case 2:
            Serial.println(" (Housekeeping)");
            break;
    }
}

// ========================================
// Setup Function
// ========================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("   OBC SIMULATOR TEST - ESP32");
    Serial.println("========================================");
    
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    
    // Print MAC address
    Serial.print("OBC MAC Address: ");
    Serial.println(WiFi.macAddress());
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("✗ Error initializing ESP-NOW");
        return;
    }
    Serial.println("✓ ESP-NOW initialized successfully");
    
    // Register callbacks
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    
    // Register CTU as peer
    memcpy(peerInfo.peer_addr, ctuMacAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("✗ Failed to add CTU peer");
        Serial.println("IMPORTANT: Did you set the correct CTU MAC address?");
        return;
    }
    Serial.println("✓ CTU peer added successfully");
    
    Serial.println("\n========================================");
    Serial.println("OBC Simulator ready!");
    Serial.println("Waiting for telecommands from CTU...");
    Serial.printf("Telemetry interval: %d ms\n", TELEMETRY_SEND_INTERVAL_MS);
    Serial.println("========================================\n");
}

// ========================================
// Loop Function
// ========================================
void loop() {
    unsigned long currentTime = millis();
    
    // Send telemetry at specified interval
    if (currentTime - last_telemetry_send >= TELEMETRY_SEND_INTERVAL_MS) {
        last_telemetry_send = currentTime;
        
        // Update placeholder telemetry values
        updatePlaceholderTelemetry();
        
        // Send different telemetry types in rotation
        esp_err_t result;
        
        switch (telemetry_cycle) {
            case 0:
                // Send GPS telemetry
                result = esp_now_send(ctuMacAddress, (uint8_t *)&gps_telemetry, sizeof(gps_telemetry));
                break;
                
            case 1:
                // Send IMU telemetry
                result = esp_now_send(ctuMacAddress, (uint8_t *)&imu_telemetry, sizeof(imu_telemetry));
                break;
                
            case 2:
                // Send Housekeeping telemetry
                result = esp_now_send(ctuMacAddress, (uint8_t *)&hk_telemetry, sizeof(hk_telemetry));
                break;
        }
        
        if (result != ESP_OK) {
            Serial.printf("✗ Error sending telemetry (cycle %d)\n", telemetry_cycle);
        } else {
            telemetry_sent_count++;
        }
        
        // Cycle through telemetry types
        telemetry_cycle = (telemetry_cycle + 1) % 3;
    }
    
    // Print status update every 5 seconds
    static unsigned long last_status_print = 0;
    if (currentTime - last_status_print >= 5000) {
        last_status_print = currentTime;
        Serial.println("\n--- Status Update ---");
        Serial.printf("Telecommands received: %d\n", telecommand_count);
        Serial.printf("Telemetry packets sent: %d\n", telemetry_sent_count);
        if (last_telecommand_received > 0) {
            Serial.printf("Last telecommand: %lu ms ago\n", currentTime - last_telecommand_received);
        }
        Serial.println("--------------------\n");
    }
    
    delay(10);  // Small delay to prevent watchdog issues
}

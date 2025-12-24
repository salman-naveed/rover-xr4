#include <SN_Common.h>
#include <SN_XR_Board_Types.h>
#include <SN_Logger.h>

// Initialize system context with safe default values
xr4_system_context_t xr4_system_context = {
    .system_state = XR4_STATE_JUST_POWERED_ON,
    
    // OBC telemetry fields - initialize to zero
    .GPS_lat = 0.0,
    .GPS_lon = 0.0,
    .GPS_time = 0.0,
    .GPS_fix = false,
    .Gyro_X = 0.0,
    .Gyro_Y = 0.0,
    .Gyro_Z = 0.0,
    .Acc_X = 0.0,
    .Acc_Y = 0.0,
    .Acc_Z = 0.0,
    .Mag_X = 0.0,
    .Mag_Y = 0.0,
    .Mag_Z = 0.0,
    .Main_Bus_V = 0.0,
    .Main_Bus_I = 0.0,
    .temp = 0.0,
    .OBC_RSSI = 0.0,
    
    // CTU control fields - initialize to safe neutral values
    .Command = 0,
    .Joystick_X = 1856,  // Neutral position (from JOYSTICK_X_NEUTRAL_DEFAULT)
    .Joystick_Y = 1880,  // Neutral position (from JOYSTICK_Y_NEUTRAL_DEFAULT)
    .Encoder_Pos = 0,
    .CTU_RSSI = 0,
    
    // Safety flags - initialize to safe state
    .Emergency_Stop = false,
    .Armed = false,
    .Headlights_On = false,
    .Buzzer = false,
    .Button_A = false,
    .Button_B = false,
    .Button_C = false,
    .Button_D = false
};

bool esp_init_success = false;

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

bool EMERGENCY_STOP_ACTIVE = false;
bool ARMED = false;
bool HEADLIGHTS_ON = false;
bool ERROR_EVENT = false;

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32




#endif

void SN_Common__Reboot()
{
    logMessage(true, "SN_Common__Reboot", "Rebooting system...");

    delay(100);
    
    ESP.restart();
}
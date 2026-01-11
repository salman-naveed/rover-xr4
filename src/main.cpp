#include <Arduino.h>
#include <SN_XR_Board_Types.h>
#include <SN_ESPNOW.h>
#include <SN_Logger.h>
#include <SN_UART_SLIP.h>
#include <SN_Handler.h>
#include <SN_LCD.h>
#include <SN_GPS.h>
#include <SN_Common.h>
#include <SN_Joystick.h>
#include <SN_Motors.h>
#include <SN_StatusPanel.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
#include <SN_Switches.h>
#elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
#include <SN_Sensors.h>
#endif

extern bool esp_init_success;
bool espnow_init_success = false; // ESP-NOW initialization status flag


extern xr4_system_context_t xr4_system_context;

const char* rr(esp_reset_reason_t r){
  switch(r){
    case ESP_RST_UNKNOWN: return "UNKNOWN";
    case ESP_RST_POWERON: return "POWERON";
    case ESP_RST_EXT:     return "EXT";
    case ESP_RST_SW:      return "SW";
    case ESP_RST_PANIC:   return "PANIC";
    case ESP_RST_INT_WDT: return "INT_WDT";
    case ESP_RST_TASK_WDT:return "TASK_WDT";
    case ESP_RST_WDT:     return "WDT";
    case ESP_RST_BROWNOUT:return "BROWNOUT";
    default: return "OTHER";
  }
}
 
void setup() {

  xr4_system_context.system_state = XR4_STATE_JUST_POWERED_ON;

  SN_UART_SLIP_Init();   // Init Serial Monitor

  logMessage(false, "Main Logger", "setup() - start");
  
  // Print firmware version information
  logMessage(false, "Firmware", "========================================");
  logMessage(false, "Firmware", "%s", FIRMWARE_FULL_NAME);
  logMessage(false, "Firmware", "Version: %s", FIRMWARE_VERSION_STRING);
  logMessage(false, "Firmware", "Build: %s %s", FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME);
  logMessage(false, "Firmware", "========================================");

  auto r = esp_reset_reason();
  logMessage(false, "Main Logger", "Last reset: %s (%d)", rr(r), (int)r);

  // SN_ESPNOW_Init();

  SN_StatusPanel__Init(); // Init Status Panel

  #if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
    SN_Switches_Init(); // Init CTU switches with interrupts and debouncing
    SN_Joystick_Init(); // Init Joystick
    SN_Input_Init(); // Init CTU Inputs (legacy buttons)
    SN_LCD_Init();  // Init LCD
  #elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
    SN_Sensors_Init(); // Init Sensors (ADC, MPU6050, DS18B20)
    SN_Motors_Init(); // Init Motors
    
    // Start background sensor reading task (IMU/MAG) - Zero latency!
    SN_OBC_StartBackgroundSensorTask();
    
    bool gps_ok = SN_GPS_Init(); // Init GPS - capture return but don't block on it
    if (!gps_ok) {
        logMessage(true, "Main Logger", "GPS init returned false - continuing without GPS");
    } else {
        logMessage(false, "Main Logger", "GPS init successful - acquiring fix in background");
    }
  #endif

  // Set initial state to INITIALIZED (ESP-NOW will be initialized in loop state machine)
  xr4_system_context.system_state = XR4_STATE_INITIALIZED;
  logMessage(false, "Main Logger", "System initialized - ESP-NOW will initialize in main loop");

  logMessage(false, "Main Logger", "setup() - end");

}

void loop() {
  // Update CTU switch states before state transitions
  // NOTE: E-STOP is handled by hardware interrupt - updates context directly
  #if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
  // Update switch debouncing (but don't read states yet - handler needs encoder_delta)
  SN_Switches_Update();
  // Note: ARM and HEADLIGHTS switches are read in SN_CTU_ControlInputsHandler()
  // which updates the context. E-STOP is updated directly by ISR.
  #endif

  // High-priority checks for safety and state transitions
  #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  if (xr4_system_context.Emergency_Stop && xr4_system_context.system_state != XR4_STATE_EMERGENCY_STOP) {
      logMessage(true, "Main Loop", "EMERGENCY STOP DETECTED - Switching to EMERGENCY_STOP state");
      SN_StatusPanel__SetStatusLedState(Blink_Red);
      SN_Motors_Stop(); // Immediately stop motors
      xr4_system_context.system_state = XR4_STATE_EMERGENCY_STOP;
  } else if (!xr4_system_context.Emergency_Stop && xr4_system_context.system_state == XR4_STATE_EMERGENCY_STOP) {
      // ESTOP released - transition back to appropriate state
      if (xr4_system_context.Armed) {
          logMessage(true, "Main Loop", "ESTOP released, ARM ON - Switching to ARMED state");
          SN_StatusPanel__SetStatusLedState(Solid_Green);
          xr4_system_context.system_state = XR4_STATE_ARMED;
      } else {
          logMessage(true, "Main Loop", "ESTOP released, ARM OFF - Switching to WAITING_FOR_ARM state");
          SN_StatusPanel__SetStatusLedState(Moving_Back_Forth);
          xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
      }
  } else if (!xr4_system_context.Armed && (xr4_system_context.system_state == XR4_STATE_ARMED || xr4_system_context.system_state == XR4_STATE_WAITING_FOR_ARM)) {
      logMessage(true, "Main Loop", "Disarmed signal received - moving to WAITING_FOR_ARM state.");
      SN_StatusPanel__SetStatusLedState(Moving_Back_Forth);
      xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
  }
  #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
  // CTU state transitions based on switch positions
  if (xr4_system_context.Emergency_Stop && xr4_system_context.system_state != XR4_STATE_EMERGENCY_STOP && xr4_system_context.system_state != XR4_STATE_ERROR) {
      logMessage(true, "Main Loop", "CTU EMERGENCY STOP DETECTED - Switching to EMERGENCY_STOP state");
      SN_StatusPanel__SetStatusLedState(Blink_Red);
      xr4_system_context.system_state = XR4_STATE_EMERGENCY_STOP;
  } else if (!xr4_system_context.Emergency_Stop && xr4_system_context.system_state == XR4_STATE_EMERGENCY_STOP) {
      // E-STOP released - return to appropriate state based on ARM switch
      if (xr4_system_context.Armed) {
          logMessage(true, "Main Loop", "CTU E-STOP released, ARM ON - Switching to ARMED state");
          SN_StatusPanel__SetStatusLedState(Solid_Green);
          xr4_system_context.system_state = XR4_STATE_ARMED;
      } else {
          logMessage(true, "Main Loop", "CTU E-STOP released, ARM OFF - Switching to WAITING_FOR_ARM state");
          SN_StatusPanel__SetStatusLedState(Moving_Back_Forth);
          xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
      }
  } else if (xr4_system_context.Armed && xr4_system_context.system_state == XR4_STATE_WAITING_FOR_ARM) {
      logMessage(true, "Main Loop", "CTU Armed - Switching to ARMED state");
      SN_StatusPanel__SetStatusLedState(Solid_Green);
      xr4_system_context.system_state = XR4_STATE_ARMED;
  } else if (!xr4_system_context.Armed && xr4_system_context.system_state == XR4_STATE_ARMED) {
      logMessage(true, "Main Loop", "CTU Disarmed - Switching to WAITING_FOR_ARM state");
      SN_StatusPanel__SetStatusLedState(Moving_Back_Forth);
      xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
  }
  #endif

  switch (xr4_system_context.system_state) {
    case XR4_STATE_INITIALIZED:
      // Perform actions for INITIALIZED state
      SN_StatusPanel__SetStatusLedState(Solid_Blue); // Initializing
      xr4_system_context.system_state = XR4_STATE_COMMS_CONFIG;
      break;

    case XR4_STATE_COMMS_CONFIG:
      if (SN_ESPNOW_Init()) {
        logMessage(true, "Main Loop", "ESP-NOW initialized successfully in COMMS_CONFIG state");
        espnow_init_success = true; // Set flag when ESP-NOW initializes successfully
        SN_StatusPanel__SetStatusLedState(Solid_Blue);
        xr4_system_context.system_state = XR4_STATE_WAITING_FOR_ARM;
      } else {
        logMessage(true, "Main Loop", "ESP-NOW initialization failed in COMMS_CONFIG state");
        espnow_init_success = false; // Clear flag on failure
        // Handle initialization failure (e.g., retry, set error state, etc.)
        xr4_system_context.system_state = XR4_STATE_ERROR;
      }
      break;

    case XR4_STATE_WAITING_FOR_ARM:
      // Perform actions for WAITING_FOR_ARM state
      SN_StatusPanel__SetStatusLedState(Moving_Back_Forth); // Waiting for arming
      #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
      SN_OBC_MainHandler();   // OBC Handler - GPS runs independently via ticker
      #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
      SN_CTU_MainHandler();   // CTU Handler
      #endif
      break;

    case XR4_STATE_ARMED: // Merged ARMED into WAITING_FOR_ARM as SN_OBC_MainHandler handles armed/disarmed logic internally
      SN_StatusPanel__SetStatusLedState(Solid_Green); // Armed and operational
      #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
      SN_OBC_MainHandler();   // OBC Handler - GPS runs independently via ticker
      
      // OPTIONAL: Enable real-time pin monitoring (only when MOTOR_DIAGNOSTIC_MODE is false)
      // Uncomment to monitor pins during operation
      // MotorDiagnostics::monitorPinsRealtime();
      
      #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
      SN_CTU_MainHandler();   // CTU Handler
      #endif
      break;

    case XR4_STATE_ERROR: {
      // Perform actions for ERROR state
      SN_StatusPanel__SetStatusLedState(Solid_Red);
      #if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
      // Update switches to enable encoder navigation even in error state
      SN_Switches_Update();
      SwitchStates_t error_state_switches = SN_Switches_GetStates();
      
      // Handle encoder navigation for LCD pages
      if (error_state_switches.encoder_delta > 0) {
        SN_LCD_NextPage();
      } else if (error_state_switches.encoder_delta < 0) {
        SN_LCD_PrevPage();
      }
      
      // Update encoder position in context (for display on CONTROL page)
      xr4_system_context.Encoder_Pos = (uint16_t)error_state_switches.encoder_position;
      
      // Update LCD to show error status even in error state
      SN_LCD_Update(&xr4_system_context);
      #elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
      SN_Motors_Stop();
      #endif
      break;
    }

    case XR4_STATE_EMERGENCY_STOP: {      // Perform actions for EMERGENCY_STOP state
      SN_StatusPanel__SetStatusLedState(Blink_Red); // Emergency stop active
      #if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
      // CRITICAL: Must still run main handler to process incoming ESP-NOW messages
      // Otherwise the OBC can never receive the command to exit ESTOP!
      // The handler will internally stop motors when Emergency_Stop flag is true
      SN_OBC_MainHandler();
      #elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
      // Update CTU display and inputs even in emergency stop
      SN_CTU_MainHandler();
      #endif
      break;
    }

    case XR4_STATE_OTA_FW_UPDATE: {
      // Perform actions for OTA_FW_UPDATE state
      SN_StatusPanel__SetStatusLedState(Blink_XR4);
      logMessage(true, "Main Loop", "Entering OTA Firmware Update mode...");
      #if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32


      #elif SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

      #endif

      // After OTA update, system should reboot or return to a safe state
      break;
    }

    case XR4_STATE_REBOOT: {      // Perform actions for REBOOT state
      SN_StatusPanel__SetStatusLedState(Blink_Yellow);
      logMessage(true, "Main Loop", "Rebooting system...");
      delay(100); // Give time for log message to send
      ESP.restart();
      break;
    }

    default:
      // Handle unknown states
      break;
  }
  
  // ULTRA LOW LATENCY: Removed delay entirely for maximum responsiveness
  // With taskYIELD(), scheduler allows other tasks to run without blocking
  // This gives sub-millisecond response time - professional competition-grade
  taskYIELD();  // Allow FreeRTOS to schedule other tasks without delay

  // The SN_StatusPanel__MainLoop() is no longer needed here
  // as it is handled by a dedicated FreeRTOS task.
}
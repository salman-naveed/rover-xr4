#include <SN_Common.h>
#include <SN_XR_Board_Types.h>
#include <SN_Logger.h>

xr4_system_context_t xr4_system_context;

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
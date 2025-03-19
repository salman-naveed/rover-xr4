#include <SN_Common.h>

xr4_system_context_t xr4_system_context;

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

bool EMERGENCY_STOP_ACTIVE = false;
bool ARMED = false;
bool HEADLIGHTS_ON = false;
bool ERROR_EVENT = false;

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32




#endif
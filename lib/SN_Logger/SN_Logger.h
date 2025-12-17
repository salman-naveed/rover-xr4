#ifndef SICON_PLUG_AI_PLATFORMIO_GPS_LOGGER_H
#define SICON_PLUG_AI_PLATFORMIO_GPS_LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

void logMessage(bool serial_verbose, String point, const char *format, ...);

void udpLog(const char *format, ...);

#endif //SICON_PLUG_AI_PLATFORMIO_GPS_LOGGER_H

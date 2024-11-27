#ifndef SICON_PLUG_AI_PLATFORMIO_GPS_LOGGER_H
#define SICON_PLUG_AI_PLATFORMIO_GPS_LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

#define POINT_1 String(__FILE__)
#define POINT_2 String(__PRETTY_FUNCTION__)
#define POINT_3 String(__LINE__)
#define POINT POINT_1+":"+POINT_2+":"+POINT_3+":"+GPS_Utils__TimestampToStr(GPS_Utils__GetCurrentTimestamp())

#define debug_log(...)  { logMessage(true, String(__FUNCTION__)+String(":")+String(__LINE__),__VA_ARGS__); }

#define debug_log_plotter(...)  { logMessage(false, String(""),__VA_ARGS__); }

void logMessage(bool serial_verbose, String point, const char *format, ...);

#define udp_log(...)  { udpLog(__VA_ARGS__); }

void udpLog(const char *format, ...);

#endif //SICON_PLUG_AI_PLATFORMIO_GPS_LOGGER_H

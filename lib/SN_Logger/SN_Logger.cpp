#include <SN_Logger.h>
#include <AsyncUDP.h>
#include <SN_WiFi.h>
// #include <GPS_Common.h>
#include <SN_Utils.h>

AsyncUDP udpClient;

uint16_t sendUdpPort = 8889;

String MAC = "";
String IP = "";

static String log_message_short = "";
static String log_message_full = "";

static String dep_log_message = "";

// TODO Examples
// debug_log("SENSOR: %5d %5d; %10llu %10llu; %5s %5s; %5d %5d",
// Plotter "$%d %d %d %d %d %d;",

String get_detailed_info(String point) {
    return String(";") +
        //    IP + String(";") +
           MAC + String(";") +
           SN_Utils__TimestampToStr(SN_Utils__GetCurrentTimestamp()) + String(";") +
           point + ";";
}

void udpLog(const char *format, ...) {
    char* log_buffer = nullptr;
    va_list arg;
    va_start(arg, format);

    int len = vsnprintf(nullptr, 0, format, arg);
    va_end(arg);
    if (len > 0) {
        log_buffer = new char[len + 1];
        va_start(arg, format);
        vsprintf(log_buffer, format, arg);
        va_end(arg);
    }

    log_message_short = String(log_buffer);

    if (SN_WiFi__IsConnectedToNetwork())
        udpClient.broadcastTo(log_message_short.c_str(), sendUdpPort);

    delete[] log_buffer;
}

void logMessage(bool serial_verbose, String point, const char *format, ...) {
#if SN_DEBUG_LOG_IS_ENABLED == 1
    char* log_buffer = nullptr;
    va_list arg;
    va_start(arg, format);

    int len = vsnprintf(nullptr, 0, format, arg);
    va_end(arg);
    if (len > 0) {
        log_buffer = new char[len + 1];
        va_start(arg, format);
        vsprintf(log_buffer, format, arg);
        va_end(arg);
    }

    log_message_short = String(log_buffer);
    // log_message_full = get_detailed_info(point) + log_message_short;

    if (serial_verbose) {
        Serial.println(log_message_short);
    } else {
        Serial.println(log_message_short);
    }
    Serial.flush();

    if (SN_WiFi__IsConnectedToNetwork())
        udpClient.broadcastTo(log_message_full.c_str(), sendUdpPort);

    delete[] log_buffer;
#endif
}

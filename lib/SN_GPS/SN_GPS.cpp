#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <SN_GPS.h>
#include <SN_Logger.h>
#include <SN_XR_Board_Types.h>
#include <Ticker.h>
#include <SN_Common.h>

// Define the serial connection to the GPS device
static const int RXPin = GPS_RX_PIN, TXPin = GPS_TX_PIN;
static const uint32_t GPS_Baud_Rate = 4800;



// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial GPS_SoftSerial(RXPin, TXPin);

Ticker gps_ticker;

Ticker gps_healthcheck_ticker;

extern xr4_system_context_t xr4_system_context;

#define GPS_DEBUG 1
#if GPS_DEBUG
  #define GPS_LOG(...) logMessage(true, __VA_ARGS__)
#else
  #define GPS_LOG(...)
#endif

unsigned long lastGPSFixTime = 0;
const unsigned long gpsTimeout = 10000; // 10 seconds

bool SN_GPS_Init() {
    // Initialize GPS data to safe defaults
    xr4_system_context.GPS_lat = 0.0;
    xr4_system_context.GPS_lon = 0.0;
    xr4_system_context.GPS_time = 0.0;
    xr4_system_context.GPS_fix = false; // Start with no fix
    lastGPSFixTime = 0; // No fix yet
    
    GPS_SoftSerial.begin(GPS_Baud_Rate);

    delay(100); // Reduced delay to 100ms instead of 1000ms

    gps_ticker.attach_ms(500, SN_GPS_Handler); // call GPS handler every 500ms
    gps_healthcheck_ticker.attach_ms(1000, checkGPSHealth); // check GPS health every second

    // Check if GPS is responding, but don't block if it isn't
    if (millis() > 5000 && gps.charsProcessed() < 10) {
        GPS_LOG("SN_GPS_Init", "No GPS detected: check wiring - continuing without GPS");
    } else {
        GPS_LOG("SN_GPS_Init", "GPS Initialized - will acquire fix in background");
    }

    return true; // Always return true to allow rover to continue
}

void SN_GPS_Handler() {
    int maxBytesToProcess = 32;
    int bytesProcessed = 0;

    while (GPS_SoftSerial.available() > 0 && bytesProcessed < maxBytesToProcess) {
        char c = GPS_SoftSerial.read();
        gps.encode(c);
        bytesProcessed++;
    }

    if (gps.location.isUpdated() || gps.time.isUpdated() || gps.date.isUpdated()) {
        SN_GPS_extractData();
    }
}

void SN_GPS_extractData() {
  if (gps.location.isValid()) {
      xr4_system_context.GPS_lat = gps.location.lat();
      xr4_system_context.GPS_lon = gps.location.lng();
  } else {
      GPS_LOG("SN_GPS_extractData", "Invalid location");
  }

  if (gps.date.isValid()) {
      // date fields could be stored separately if needed
  } else {
      GPS_LOG("SN_GPS_extractData", "Invalid date");
  }

  if (gps.time.isValid()) {
      xr4_system_context.GPS_time = gps.time.hour() * 3600 +
                                    gps.time.minute() * 60 +
                                    gps.time.second() +
                                    gps.time.centisecond() / 100.0;
  } else {
      GPS_LOG("SN_GPS_extractData", "Invalid time");
  }

  bool fix = gps.location.isValid() && gps.date.isValid() && gps.time.isValid();
  xr4_system_context.GPS_fix = fix;

  if (fix) {
      lastGPSFixTime = millis();
  }

  GPS_LOG("SN_GPS_extractData", "Lat: %f, Lon: %f, Time: %f, Fix: %d",
           xr4_system_context.GPS_lat,
           xr4_system_context.GPS_lon,
           xr4_system_context.GPS_time,
           xr4_system_context.GPS_fix);
}

void checkGPSHealth() {
    if (millis() - lastGPSFixTime > gpsTimeout && lastGPSFixTime > 0) {
        GPS_LOG("GPS Watchdog", "GPS signal lost or timed out");
        xr4_system_context.GPS_fix = false;
    }
}


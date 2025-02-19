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

GPSData_t gps_data;

extern xr4_system_context_t xr4_system_context; 



bool SN_GPS_Init() {
    GPS_SoftSerial.begin(GPS_Baud_Rate);

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      logMessage(true, "SN_GPS_Init", "No GPS detected: check wiring.");
      return false;
    }

    logMessage(true, "SN_GPS_Init", "GPS Initialized");
    return true;
}


void SN_GPS_Handler() {
    // This sketch displays information every time a new sentence is correctly encoded.
    while (GPS_SoftSerial.available() > 0){
        if (gps.encode(GPS_SoftSerial.read())){
          SN_GPS_extractData();
        }
    }
}


void SN_GPS_extractData() {
  // Extract location (latitude and longitude)
  if (gps.location.isValid()) {
    gps_data.isValidLocation = true;
    gps_data.latitude = gps.location.lat();
    gps_data.longitude = gps.location.lng();
  } else {
    gps_data.isValidLocation = false;
  }

  // Extract date (day, month, year)
  if (gps.date.isValid()) {
    gps_data.isValidDate = true;
    gps_data.day = gps.date.day();
    gps_data.month = gps.date.month();
    gps_data.year = gps.date.year();
  } else {
    gps_data.isValidDate = false;
  }

  // Extract time (hour, minute, second, centisecond)
  if (gps.time.isValid()) {
    gps_data.isValidTime = true;
    gps_data.hour = gps.time.hour();
    gps_data.minute = gps.time.minute();
    gps_data.second = gps.time.second();
    gps_data.centisecond = gps.time.centisecond();
  } else {
    gps_data.isValidTime = false;
  }
}


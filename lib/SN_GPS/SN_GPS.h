
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

typedef struct GPSData {
  bool isValidLocation;
  double latitude;
  double longitude;
  bool isValidDate;
  int day;
  int month;
  int year;
  bool isValidTime;
  int hour;
  int minute;
  int second;
  int centisecond;
} GPSData_t;



bool SN_GPS_Init();
void SN_GPS_Handler();
void SN_GPS_extractData();

void checkGPSHealth();
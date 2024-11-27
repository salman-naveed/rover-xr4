#include <SN_SDCard.h>
#include <SN_Logger.h>
#include <SN_GPS.h>

String log_file_name = "/log_data.txt";
String log_file_headings = "Date, Time, GPS_Lat, GPS_Long, Main_Bus_V, Main_Bus_I, Speed, Heading_deg \r\n";


void SN_SDCard_Init() {
    // Initialize SD card
    SD.begin(SD_CS);  
    if(!SD.begin(SD_CS)) {
        logMessage(true, "SN_SDCard_Init", "SD Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE) {
        logMessage(true, "SN_SDCard_Init", "No SD card attached");
        return;
    }
        logMessage(true, "SN_SDCard_Init", "Initializing SD card...");
    if (!SD.begin(SD_CS)) {
        logMessage(true, "SN_SDCard_Init", "SD card initialization failed");
        return;    // init failed
    }
}

void SN_SDCard_FileInit(String log_file_name) {
    // Open file for writing
    File file = SD.open(log_file_name, FILE_WRITE);
    if(!file) {
        logMessage(true, "SN_SDCard_FileInit", "Failed to open file for writing / File doesn't exist");
        logMessage(true, "SN_SDCard_FileInit", "Creating file...");
        writeFile(SD, log_file_name.c_str(), log_file_headings.c_str());
        return;
    }
    else {
        logMessage(true, "SN_SDCard_FileInit", "File already exists");  
    }
    // Close the file
    file.close();
}

void SN_SDCard_Write(String log_file_name, String log_data) {
    // Open file for writing
    logMessage(true, "SN_SDCard_Write", "Writing data to file: %s", log_data.c_str());

    writeFile(SD, log_file_name.c_str(), log_data.c_str());
}


// Write the data on the SD card
void SN_SDCard_Log(String log_file_name, String log_data) {
    // Open file for writing
    logMessage(true, "SN_SDCard_Log", "Appending data to file: %s", log_data.c_str());

    appendFile(SD, log_file_name.c_str(), log_data.c_str());
  
    // String dataMessage = String(readingID) + "," + String(dayStamp) + "," + String(timeStamp) + "," + 
    //             String(temperature) + "\r\n";

}

void SN_SDCard_MainDataLogger(String log_file_name, GPSData gps_data, ) {
    // Open file for writing
    logMessage(true, "SN_SDCard_MainDataLogger", "Appending data to file: %s", log_data.c_str());

    appendFile(SD, log_file_name.c_str(), log_data.c_str());
}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char * path, const char * message) {
    logMessage(true, "SN_SDCard_Write", "Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file) {
        logMessage(true, "SN_SDCard_Write", "Failed to open file for writing");
    return;
    }
    if(file.print(message)) {
        logMessage(true, "SN_SDCard_Write", "File written");
    } else {
        logMessage(true, "SN_SDCard_Write", "Write failed");
    }
    file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    logMessage(true, "SN_SDCard_Write", "Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    logMessage(true, "SN_SDCard_Write", "Message appended");
  } else {
    logMessage(true, "SN_SDCard_Write", "Append failed");
  }
  file.close();
}
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Define CS pin for the SD card module
#define SD_CS 5

void SN_SDCard_Init();
void SN_SDCard_FileInit(String filename);
void SN_SDCard_Write(String filename, String data);

void SN_SDCard_Read(String filename);
void SN_SDCard_Delete(String filename);
void SN_SDCard_ListFiles();
void SN_SDCard_DeleteAllFiles();
void SN_SDCard_WriteFile(String filename, String data);
void SN_SDCard_ReadFile(String filename);
void SN_SDCard_DeleteFile(String filename);

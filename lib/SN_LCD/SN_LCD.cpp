#ifndef SN_LCD_H
#define SN_LCD_H

#if SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <SN_LCD.h>
#include <SN_Logger.h>

#define LCD_ADDRESS 0x27
#define LCD_ROWS 4
#define LCD_COLUMNS 20
#define LCD_BACKLIGHT 255

extern bool espnow_init_success; // Flag to check if ESP-NOW is initialized

byte lcd_startup_char[8] =  { 
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

LiquidCrystal_PCF8574 lcd(LCD_ADDRESS);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void SN_LCD_Init(){
    Wire.begin();

    Wire.available();

    Wire.beginTransmission(LCD_ADDRESS);
    uint8_t error = Wire.endTransmission();

    logMessage(true, "SN_LCD_Init", "Scanning for LCD at address %d", LCD_ADDRESS);

    if (error == 0) {
        logMessage(true, "SN_LCD_Init", "LCD found at address %d", LCD_ADDRESS);
        int show = 0;
        lcd.begin(LCD_COLUMNS,LCD_ROWS);  // initialize the lcd
        lcd.setBacklight(LCD_BACKLIGHT); // set the backlight on
        
        SN_LCD_PrintAt(5, 0, "ROVER XR-4");
        SN_LCD_PrintAt(0, 2, "Connecting...");

        delay(1000); // Wait for a second before showing the next message

        SN_LCD_Clear();

        if(espnow_init_success) {
            SN_LCD_PrintAt(0, 2, "ESP-NOW Connected");
            SN_LCD_PrintAt(0, 3, "Ready to go!");
        } else {
            SN_LCD_PrintAt(0, 2, "ESP-NOW Not Ready");
            SN_LCD_PrintAt(0, 3, "Retrying... ");
        }

        lcd.createChar(1, lcd_startup_char);
    

    } else {
        logMessage(true, "SN_LCD_Init", "LCD not found at address %d, Error: %d", LCD_ADDRESS, error);
    }  


}

void SN_LCD_Clear(){
  lcd.clear();
}

void SN_LCD_Print(String message){
  lcd.clear();
  lcd.print(message);
}

void SN_LCD_PrintAt(int col, int row, String message){
  lcd.setCursor(col, row);
  lcd.print(message);
}

void SN_LCD_PrintAt(int col, int row, int message){
  lcd.setCursor(col, row);
  lcd.print(message);
}

void SN_LCD_PrintAt(int col, int row, float message){
  lcd.setCursor(col, row);
  lcd.print(message);
}

void SN_LCD_PrintAt(int col, int row, double message){
  lcd.setCursor(col, row);
  lcd.print(message);
}   


#endif
#endif
#include <Preferences.h> //library for storing variables in flash memory
#include "SN_Preferences.h"



//preferences library allows you to store variable values within the flash so that their value can be retrieved on startup. Used to store user-adjustable settings.
Preferences preferences;

void SN_Preferences_begin() {
    bool preferences_initialized = preferences.begin("SN_Preferences", false, "Defaults"); //open the preferences library
    if (!preferences_initialized) {
        Serial.println("Preferences initialization failed");
    }
    else {
        Serial.println("Preferences initialized");
    }
}

void SN_Preferences_end() {
    preferences.end(); //close the preferences library
}

void SN_Preferences_clear() {
    preferences.clear(); //clear all stored preferences
}

void SN_Preferences_remove(const char* key) {
    preferences.remove(key); //remove a specific preference
}

void SN_Preferences_putChar(const char* key, int8_t value) {
    preferences.putChar(key, value); //store a char value
}

void SN_Preferences_putUChar(const char* key, uint8_t value) {
    preferences.putUChar(key, value); //store an unsigned char value
}

void SN_Preferences_putShort(const char* key, int16_t value) {
    preferences.putShort(key, value); //store a short value
}


void SN_Preferences_putUShort(const char* key, uint16_t value) {
    preferences.putUShort(key, value); //store an unsigned short value
}

void SN_Preferences_putInt(const char* key, int32_t value) {
    preferences.putInt(key, value); //store an int value
}

void SN_Preferences_putUInt(const char* key, uint32_t value) {
    preferences.putUInt(key, value); //store an unsigned int value
}

void SN_Preferences_putLong(const char* key, int32_t value) {
    preferences.putLong(key, value); //store a long value
}

void SN_Preferences_putULong(const char* key, uint32_t value) {
    preferences.putULong(key, value); //store an unsigned long value
}


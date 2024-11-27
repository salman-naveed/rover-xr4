
#include <Arduino.h>
#include <WiFi.h>
#include <SN_WiFi.h>
#include <SN_Logger.h>

extern String MAC;


void SN_WiFi_StartAsWiFiClient() {

    // StartWiFiWatchdog();

    WiFi.mode(WIFI_STA);

    delay(10);

    MAC = WiFi.macAddress();

    // connectToWiFi();

}

bool SN_WiFi__IsConnectedToNetwork() {
    return WiFi.isConnected();
}

void connectToWiFi() {
    const int wifiTimeout = 5000;
    unsigned long startAttemptTime = millis();

    if(WiFi.status() == WL_CONNECTED) {
        logMessage(true, "connectToWiFi", "Already connected to WiFi network: %s", WiFi.SSID().c_str());
        return;
    }
    else if (!SN_WiFi__IsConnectedToNetwork())
    {
        logMessage(true, "connectToWiFi", "Not connected to WiFi network: %s", DEFAULT_WIFI_SSID);
        // Connect to Wi-Fi network with SSID and password
        logMessage(true, "connectToWiFi", "Connecting to WiFi network: %s", DEFAULT_WIFI_SSID);

        WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASSWORD);

        // Wait for connection
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
            delay(100);
            logMessage(true, "connectToWiFi", "Connecting to WiFi network: %s", DEFAULT_WIFI_SSID);
        }

        logMessage(true, "connectToWiFi", "Connected to WiFi network: %s", DEFAULT_WIFI_SSID);
        logMessage(true, "connectToWiFi", "IP address: %s", WiFi.localIP().toString().c_str());
        logMessage(true, "connectToWiFi", "MAC address: %s", WiFi.macAddress().c_str());
    }

}
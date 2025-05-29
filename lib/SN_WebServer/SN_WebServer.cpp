#if SN_WEBSERVER_IS_ENABLED == 1

#include "SN_WebServer.h"
#include <Arduino.h>
#include <ESP32WebServer.h>
#include <HTTPClient.h>
#include <SN_Logger.h>
#include <ArduinoJson.h>
#include <SN_Utils.h>

ESP32WebServer roverServer(80);

String getClientIp() {
    WiFiClient client = roverServer.client();

    if (client) {
        return String(client.remoteIP().toString().c_str());
    }

    return "Unknown";
}

void SendJsonMessage(String json, int httpCode) {
    roverServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    roverServer.send(httpCode, "text/json", json);
}

void SendHTMLMessage(String html, int httpCode) {
    roverServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    roverServer.send(httpCode, "text/html", html);
}

void SendJsonStatus(int httpCode) {
    String status = httpCode == HTTP_CODE_OK ? "OK" : "Error";

    String json = String();
    json = "{";
    json += String("\"status\":\"") + status + "\"";
    json += "}";

    SendJsonMessage(json, httpCode);
}

void IndexPage_Handler() {
    String html = 
        "<html>"
        "<head>"
        "<title>XR-4 Rover</title>"
        "</head>"
        "<body>"
        "<h1>Welcome to the XR-4 Rover Web Server</h1>"
        "<p>IP Address: " + WiFi.localIP().toString() + "</p>"
        "<p>MAC Address: " + WiFi.macAddress() + "</p>"
        "</body>"
        "</html>";

    SendHTMLMessage(html, HTTP_CODE_OK);
}

void DeviceStatus_Handler() {
    String clientIP = getClientIp();
    // debug_log_deprecated(String("Handler '/status' - begin, from: ") + clientIP);

    DynamicJsonBuffer jsonBufferWrite(MAX_JSON_MESSAGE_SIZE);
    JsonObject &root = jsonBufferWrite.createObject();

    JsonObject &status = root.createNestedObject("Status");
    status[F("free_heap")] = SN_Utils__FormatBytes(ESP.getFreeHeap());
//    status[F("Host_Device_Status")] = int(g_context.device_status);
//    status[F("Hostname")] = hostName;


    JsonObject &timeInfo = root.createNestedObject("TimeInfo");
    timeInfo[F("millis_64")] = SN_Utils__TimestampToStr(SN_Utils__Millis64());
    timeInfo[F("date_utc")] = SN_Utils__FormatCurrentDateTime();
    timeInfo[F("last_sync_utc")] = SN_Utils__LastUpdateDateTime();


    JsonObject &deviceInfo = root.createNestedObject("PlugInfo");
    deviceInfo[F("product_name")] = PLUG_PRODUCT_NAME;
    deviceInfo[F("v_id")] = PLUG_VENDOR_ID;
    deviceInfo[F("d_id")] = PLUG_DEVICE_ID;
    deviceInfo[F("SN")] = GPS_EEPROM_Settings__Get_SerialNumber();
    deviceInfo[F("UID")] = GPS_Utils__GetUniqueID();
    deviceInfo[F("WiFi_MAC")] = GPS_Configuration__GetWiFiMAC();
    deviceInfo[F("Ethernet_MAC")] = GPS_Configuration__GetEthernetMAC();
    deviceInfo[F("CM_Events")] = (int) eventSender.Get_ConditionMonitoring_Events();
    deviceInfo[F("Communication")] = uint16_t(GPS_Configuration__GetConnectionMode()) ? "LAN" : "WLAN";


    JsonObject &version = root.createNestedObject(F("VersionInfo"));
    version[F("OTA_support")] = true;
    version[F("version")] = PLUG_FW_VERSION.c_str();
    version[F("build")] = GPS_Utils__FormatFirmwareBuildDateTime();
    version[F("b1")] = g_GPS_Sensor.product_info.HC_bit_1;
    version[F("b2")] = g_GPS_Sensor.product_info.HC_bit_2;
    version[F("b3")] = g_GPS_Sensor.product_info.HC_bit_3;
    version[F("real_ver")] = g_GPS_Sensor.product_info.real_ver;
    version[F("hw_v")] = g_GPS_Sensor.product_info.hw_v;
    version[F("case_id")] = (int) g_GPS_Sensor.product_info.device_case;

    JsonObject &espnowInfo = root.createNestedObject("ESP-NOW");
    espnowInfo[F("device_id")] = 
    espnowInfo[F("device_mac")] = 
    espnowInfo[F("peer_count")] = 

    espnowInfo[F("peer_mac")] =
    espnowInfo[F("peer_id")] =
    espnowInfo[F("peer_channel")] =

    #if XR4_BOARD_TYPE == XR4_OBC_ESP32

    JsonObject &GPSInfo = root.createNestedObject("GPS");
    GPSInfo[F("latitude")] = g_GPS_Sensor.gps_data.latitude;
    GPSInfo[F("longitude")] = g_GPS_Sensor.gps_data.longitude;
    GPSInfo[F("altitude")] = g_GPS_Sensor.gps_data.altitude;
    GPSInfo[F("speed")] = g_GPS_Sensor.gps_data.speed;
    GPSInfo[F("satellites")] = g_GPS_Sensor.gps_data.satellites;
    GPSInfo[F("time")] = g_GPS_Sensor.gps_data.time;
    GPSInfo[F("date")] = g_GPS_Sensor.gps_data.date;
    GPSInfo[F("isValidLocation")] = g_GPS_Sensor.gps_data.isValidLocation;

    JsonObject &IMUInfo = root.createNestedObject("IMU");
    IMUInfo[F("acc_x")] = g_GPS_Sensor.imu_data.acc_x;
    IMUInfo[F("acc_y")] = g_GPS_Sensor.imu_data.acc_y;
    IMUInfo[F("acc_z")] = g_GPS_Sensor.imu_data.acc_z;
    IMUInfo[F("gyro_x")] = g_GPS_Sensor.imu_data.gyro_x;
    IMUInfo[F("gyro_y")] = g_GPS_Sensor.imu_data.gyro_y;
    IMUInfo[F("gyro_z")] = g_GPS_Sensor.imu_data.gyro_z;
    IMUInfo[F("mag_x")] = g_GPS_Sensor.imu_data.mag_x;
    IMUInfo[F("mag_y")] = g_GPS_Sensor.imu_data.mag_y;
    IMUInfo[F("mag_z")] = g_GPS_Sensor.imu_data.mag_z;

    JsonObject &HKInfo = root.createNestedObject("Housekeeping");
    HKInfo[F("main_bus_v")] = g_GPS_Sensor.hk_data.main_bus_v;
    HKInfo[F("main_bus_i")] = g_GPS_Sensor.hk_data.main_bus_i;
    HKInfo[F("temp")] = g_GPS_Sensor.hk_data.temp;
    HKInfo[F("rssi")] = g_GPS_Sensor.hk_data.rssi;


    #endif





    JsonObject &wifi = storage.createNestedObject("WIFI");
    wifi[F("WIFI_IsEnabled")] = eepromValues->WIFI_IsEnabled;
    wifi[F("WIFI_DHCP_IsEnabled")] = eepromValues->WIFI_DHCP_IsEnabled;
    wifi[F("WIFI_STATIC_IP")] = eepromValues->WIFI_STATIC_IP;
    wifi[F("WIFI_STATIC_Subnet")] = eepromValues->WIFI_STATIC_Subnet;
    wifi[F("WIFI_STATIC_Gateway")] = eepromValues->WIFI_STATIC_Gateway;
    wifi[F("WIFI_STATIC_DNS")] = eepromValues->WIFI_STATIC_DNS;
    // wifi[F("WiFI_SSID")] = eepromValues->WiFI_SSID;
    // wifi[F("WiFI_Password")] = eepromValues->WiFI_Password;




    JsonObject &eMQTT = root.createNestedObject("eMQTT");
    eMQTT[F("Broker_Communication_Status")] = g_context.plugin_is_connected_to_the_broker
	? String("connected")
	: String(F("disconnected"));
    eMQTT[F("plug_running")] = g_context.plug_is_running;
    eMQTT[F("devices_running")] = g_context.devices_are_running;
    eMQTT[F("events")] = g_context.event.sending_events_is_allowed["main"];
    eMQTT[F("Mode")] = GPS_EEPROM_Settings__Get_Working_Mode();
    eMQTT[F("MQTT_HOST_PreRegMode")] = g_context.PreRegMode_MQTT_Server;
    eMQTT[F("MQTT_HOST_RegMode")] = GPS_EEPROM_Settings__Get_RegModeMQTT_Server();
    eMQTT[F("MQTT_PORT")] = mqttPort;
    eMQTT[F("ws_connected")] = g_context.is_ws_connected;

    eMQTT[F("UID")] = GPS_Utils__GetUniqueID();
    String wifimac = GPS_Configuration__GetWiFiMAC();
    wifimac.toUpperCase();
    eMQTT[F("MAC")] = wifimac;

    eMQTT[F("h_e")] = eventSender.has_errors();
    eMQTT[F("h_w")] = eventSender.has_warnings();

#if GPS_PRODUCT_TYPE == SICON_PLUG_PICKBEAM
    JsonObject &pickbeam = root.createNestedObject("Worker Assistance");
    pickbeam[F("URL")] = GPS_Index__Storage__GetText_by_Index(6000);
    pickbeam[F("Username")] = GPS_Index__Storage__GetText_by_Index(6001);
    String maskedpass = "";
    for (uint8_t i = 0; i < GPS_Index__Storage__GetText_by_Index(6002).length(); i++) {
        maskedpass += "*";
    }
    pickbeam[F("Password")] = maskedpass;
    pickbeam[F("Status")] = GPS_Index__Storage__GetText_by_Index(6003);
#endif

    JsonObject &httpStat = root.createNestedObject(F("HTTP_Stats"));
    httpStat[F("Count_set_broker_host")] = total_count_set_broker_host;
    httpStat[F("Count_status")] = total_count_status;

    GPS_Common_ExecCycle_Struct metricsValues = GPS_Common__GetExecCycle();
    JsonObject &metrics = root.createNestedObject("Metrics");
    metrics[F("main_loop_avg_duration_us")] = GPS_Utils__U64ToDecString(
            metricsValues.OneCycleAvg[GPS_COMMON_MAIN_LOOP_PROFILE_INDEX]);
//    metrics[F("input_handle_avg_duration_us")] = GPS_Utils__U64ToDecString(
//            metricsValues.OneCycleAvg[GPS_COMMON_INPUT_HANDLE_INDEX]);
//    metrics[F("before_data_transfer_avg_duration_us")] = GPS_Utils__U64ToDecString(
//            metricsValues.OneCycleAvg[GPS_COMMON_BEFORE_DATA_HANDLE_INDEX]);


    String json = String();
    json = "";
    root.prettyPrintTo(json);

    SendJsonMessage(json, HTTP_CODE_OK);

    // debug_log_deprecated(String("Handler '/status' - end, from: ") + clientIP);
}

void optionHandler(HTTPMethod method) {
    roverServer.sendHeader(F("Access-Control-Max-Age"), F("10000"));

    if (method == HTTP_GET) {
        roverServer.sendHeader(F("Access-Control-Allow-Methods"), F("GET,OPTIONS"));
    } else if (method == HTTP_POST) {
        roverServer.sendHeader(F("Access-Control-Allow-Methods"), F("POST,OPTIONS"));
    } else if (method == HTTP_PUT) {
        roverServer.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,OPTIONS"));
    } else if (method == HTTP_PATCH) {
        roverServer.sendHeader(F("Access-Control-Allow-Methods"), F("PATCH,OPTIONS"));
    } else if (method == HTTP_DELETE) {
        roverServer.sendHeader(F("Access-Control-Allow-Methods"), F("DELETE,OPTIONS"));
    } else {
        roverServer.sendHeader(F("Access-Control-Allow-Methods"), F("GET,POST,PUT,PATCH,DELETE,OPTIONS"));
    }

    roverServer.sendHeader(F("Access-Control-Allow-Headers"), F("Origin, X-Requested-With, Content-Type, Accept"));
    roverServer.sendHeader(F("Access-Control-Allow-Origin"), F("*"));

    roverServer.send(200, F("text/plain"), "");
}

void addWebRoute(const String &uri, HTTPMethod method, ESP32WebServer::THandlerFunction fn) {
    roverServer.on(uri, method, fn);
    roverServer.on(uri, HTTP_OPTIONS, [method]() { optionHandler(method); });
}

void SN_WebServer_Init() {
    // Initialize the web server
    addWebRoute(F("/"), HTTP_GET, IndexPage_Handler);
    addWebRoute(F("/status"), HTTP_GET, DeviceStatus_Handler);

    addWebRoute(F("/__reboot__"), HTTP_GET, Reboot_Handler);
    addWebRoute(F("/__http_ota__"), HTTP_POST, HTTP_OTA_Handler);
    addWebRoute(F("/__factory_reset_with_serial__"), HTTP_POST, FactoryResetWithSerial_Handler);
    addWebRoute(F("/__erase_eeprom_keep_serial__"), HTTP_POST, EraseEEPROMKeepSerial_Handler);

    addWebRoute(F("/get_eeprom_indexes_from_ram"), HTTP_GET, Get_EEPROM_Indexes_FROM_RAM_Handler);
    addWebRoute(F("/get_eeprom_indexes_from_eeprom"), HTTP_GET, Get_EEPROM_Indexes_FROM_EEPROM_Handler);
    addWebRoute(F("/indexes"), HTTP_GET, Get_All_Indexes_FROM_RAM_Handler);

    addWebRoute(F("/set_broker_host"), HTTP_POST, SetMQTTBrokerHost_Handler);

    addWebRoute(F("/get_identification_parameters"), HTTP_GET, GetIdentificationParameters_Handler);
    addWebRoute(F("/set_communication_parameters"), HTTP_POST, SetCommunicationParameters_Handler);
    addWebRoute(F("/get_description_of_additional_parameters"), HTTP_GET, GetDescriptionOfAdditionalParameters_Handler);
    addWebRoute(F("/set_additional_parameters"), HTTP_POST, SetAdditionalParameters_Handler);

    addWebRoute(F("/perform_command"), HTTP_POST, PerformCommand_Handler);

    roverServer.on("/", []() {
        roverServer.send(200, "text/html", "<h1>Welcome to the XR4 Rover Web Server</h1>");
    });

    roverServer.on("/status", []() {
        String status = "<h1>XR4 Rover Status</h1>";
        status += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
        status += "<p>MAC Address: " + WiFi.macAddress() + "</p>";
        roverServer.send(200, "text/html", status);
    });

    roverServer.begin();
    logMessage(true, "SN_WebServer_Init", "Web server started");
}

void SN_WebServer_handleClient() {
    roverServer.handleClient();
}

void WiFiEvent(WiFiEvent_t event) {
    String id = "";

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            id = "ARDUINO_EVENT_WIFI_READY";
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            id = "ARDUINO_EVENT_WIFI_SCAN_DONE";
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            id = "ARDUINO_EVENT_WIFI_STA_START";
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            id = "ARDUINO_EVENT_WIFI_STA_STOP";
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            id = "ARDUINO_EVENT_WIFI_STA_CONNECTED";
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            id = "ARDUINO_EVENT_WIFI_STA_DISCONNECTED";
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            id = "ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE";
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            id = "ARDUINO_EVENT_WIFI_STA_GOT_IP";
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            id = "ARDUINO_EVENT_WIFI_STA_GOT_IP6";
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            id = "ARDUINO_EVENT_WIFI_STA_LOST_IP";
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            id = "ARDUINO_EVENT_WIFI_AP_START";
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            id = "ARDUINO_EVENT_WIFI_AP_STOP";
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            id = "ARDUINO_EVENT_WIFI_AP_STACONNECTED";
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            id = "ARDUINO_EVENT_WIFI_AP_STADISCONNECTED";
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            id = "ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED";
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            id = "ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED";
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            id = "ARDUINO_EVENT_WIFI_AP_GOT_IP6";
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            id = "ARDUINO_EVENT_WPS_ER_SUCCESS";
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            id = "ARDUINO_EVENT_WPS_ER_FAILED";
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            id = "ARDUINO_EVENT_WPS_ER_TIMEOUT";
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            id = "ARDUINO_EVENT_WPS_ER_PIN";
            break;
        default:
            id = "Unknown Event";
            break;
    }

    logMessage(false, "Configuration Base", "WiFi event(%i): %s", event, id.c_str());
}

#endif // SN_WEBSERVER_IS_ENABLED
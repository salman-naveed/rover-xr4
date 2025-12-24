#include <SN_Utils.h>
#include <RTClib.h>
#include <WiFi.h>
#include <SN_Logger.h>
#include <sys/time.h>
#include <chrono>


uint64_t lastDataTimeStampFromBroker_utc = 0;

int setUnixTime(int32_t unix_time) {
    timeval epoch = {unix_time, 0};
    return settimeofday((const timeval *) &epoch, 0);
}

uint64_t SN_Utils__Millis64() {
    return SN_Utils__GetCurrentTimestamp();
}

String formatDate(DateTime dateTime);
String formatDateShort(DateTime dateTime);

String formatDateVersion(DateTime dateTime);

String urldecode(String str);

String urlencode(String &str);

unsigned char h2int(char c);


void SN_Utils__SetCurrentTime(uint64_t timestamp) {
    if (!timestamp) return;

    setUnixTime(static_cast<int32_t>(timestamp / 1000));

    lastDataTimeStampFromBroker_utc = timestamp;
}

uint64_t SN_Utils__GetCurrentTimestamp() {
    // return (((uint64_t) (SN_Utils__GetNow())) * 1000) + ((uint64_t) ((esp_timer_get_time() / 1000) % 1000));
    //
    // struct timeval tp;
    // gettimeofday(&tp, NULL);
    // return (uint64_t) (tp.tv_sec * 1000 + tp.tv_usec / 1000);
    //
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

long SN_Utils__GetNow() {
    time_t now;
    time(&now);

    return now;
}

String SN_Utils__FormatBytes(size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + "B";
    } else if (bytes < (1024 * 1024)) {
        return String(bytes / 1024.0) + "KB";
    } else if (bytes < (1024 * 1024 * 1024)) {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    } else {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

String SN_Utils__Get_HEX_Values_WithCheckNull(byte *array, int len) {
    String str = "";
    for (int i = 0; i < len; ++i) {
        if (array[i] == '\0') break;

        str += SN_Utils__ToHex(array[i]);
    }

    return str;
}

String SN_Utils__StringToHex(String value) {
    if (value.length() == 0) {
        return "";
    }

    String str;
    str.reserve(value.length() * 2); // Reserve enough space for the hex string

    for (char i : value) {
        str += SN_Utils__ToHex(static_cast<byte>(i));
    }

    return str;
}

String SN_Utils__ToHex(byte value) {
    char buf[3];
    sprintf(buf, "%02X", value);
    return String(buf);
}

String SN_Utils__ByteArrayToFormatString(byte array[], unsigned int len) {
    if (len == 0) return "";

    String str;
    str.reserve(len * 3); // 2 for hex characters, 1 for space

    for (unsigned int i = 0; i < len; i++) {
        str += SN_Utils__ToHex(array[i]) + " ";
    }

    str.remove(str.length() - 1); // Remove the trailing space
    return str;
}


String SN_Utils__ByteArrayToHexString(byte array[], unsigned int len) {
    String str;
    str.reserve(len * 2); // Each byte converts to 2 hex characters

    for (unsigned int i = 0; i < len; i++) {
        str += SN_Utils__ToHex(array[i]);
    }

    return str;
}

String SN_Utils__ByteArrayToString(byte array[], unsigned int len) {
    String str;
    str.reserve(len * 2); // Each byte converts to 2 hex characters

    for (unsigned int i = 0; i < len; i++) {
        str += SN_Utils__ToHex(array[i]);
    }

    str.toLowerCase();
    return str;
}

String SN_Utils__ByteArrayToString(byte array[], uint16_t start_index, uint16_t len) {
    String str;
    str.reserve(len * 2); // Each byte converts to 2 hex characters

    for (uint16_t i = start_index; i < (start_index + len); i++) {
        str += SN_Utils__ToHex(array[i]);
    }

    str.toLowerCase();
    return str;
}

String SN_Utils__ByteArrayToString_Format(byte array[], uint16_t start_index, uint16_t len) {
    if (len == 0) return "";

    String str;
    str.reserve(len * 3); // 2 for hex, 1 for space

    for (uint16_t i = start_index; i < (start_index + len); i++) {
        str += SN_Utils__ToHex(array[i]) + " ";
    }

    str.remove(str.length() - 1); // Remove the trailing space
    str.toUpperCase();
    return str;
}

String SN_Utils__ByteArrayToHexString(byte array[], int16_t start_index, unsigned int len) {
    String str;
    str.reserve(len * 2); // Each byte converts to 2 hex characters

    for (int16_t i = start_index; i < (start_index + len); i++) {
        str += SN_Utils__ToHex(array[i]);
    }

    return str;
}

String SN_Utils__DecByteArrayToString(byte array[], unsigned int len) {
    String str;
    str.reserve(len); // Reserve space for each character

    for (unsigned int i = 0; i < len; i++) {
        str += static_cast<char>(array[i]);
    }

    return str;
}

String SN_Utils__DecByteArrayToString(byte array[], int16_t start_index, unsigned int len) {
    // Add boundary checks if necessary

    String str;
    str.reserve(len); // Reserve space for each character

    for (int16_t i = start_index; i < (start_index + len); i++) {
        str += static_cast<char>(array[i]);
    }

    return str;
}

uint64_t SN_Utils__HexStringToU64(String str) {
    uint64_t val = 0;

    for (int i = 0; i < str.length(); i++) {
        val = val * 16;
        val = val + (int) strtol(str.substring(i, static_cast<unsigned int>(i + 1)).c_str(), nullptr, 16);
    }

    return val;
}

String SN_Utils__HexStringToDecString(String str) {
    if (str.length() == 0) return "0";

    return SN_Utils__U64ToDecString(SN_Utils__HexStringToU64(str));
}

void SN_Utils__HexStringToByteArray(String value, byte *array) {
    int len = value.length() / 2;

    for (int i = 0; i < len; i++) {
        String sub = value.substring(i * 2, i * 2 + 2);

        byte byte_value = static_cast<byte>(strtol(sub.c_str(), nullptr, 16));
        array[i] = byte_value;
    }
}

String SN_Utils__U64ToHexString(uint64_t value) {
    char buf[17]; // Maximum size for a 64-bit hex number is 16 characters plus null terminator

    sprintf(buf, "%016llX", value);

    String ret = String(buf);
    return ret;
}

String SN_Utils__U64ToDecString(uint64_t value) {
    char buf[21]; // Maximum length of a 64-bit number in decimal format plus null terminator

    sprintf(buf, "%llu", value);

    return String(buf);
}

unsigned int SN_Utils__GetBuildTimeStamp() {
    DateTime dt = DateTime(__DATE__, __TIME__);

    return dt.unixtime();
}

// Format [DD.MM.YYYY]
String SN_Utils__FormatCurrentDate() {
    if (lastDataTimeStampFromBroker_utc == 0)
        return String("00/00/0000");

    DateTime dateTime(static_cast<uint32_t>(SN_Utils__GetNow()));

    return formatDateShort(dateTime);
}

// Format [DD.MM.YYYY hh:mm:ss]
String SN_Utils__FormatCurrentDateTime() {
    if (lastDataTimeStampFromBroker_utc == 0)
        return String("00.00.0000 00:00:00");

    DateTime dateTime(static_cast<uint32_t>(SN_Utils__GetNow()));

    return formatDate(dateTime);
}

String SN_Utils__FormatDateTime(uint32_t value) {
    if (value == 0)
        return String("00.00.0000 00:00:00");

    DateTime dateTime(static_cast<uint32_t>(value));

    return formatDate(dateTime);
}

void SN_Utils__UpdateSystemMills() {
    SN_Utils__Millis64();
}

String SN_Utils__TimestampToStr(uint64_t timestamp) {
    char data[24] = {0};
    sprintf(data, "%llu", timestamp);

    return String(data);
}

String SN_Utils__FormatCurrentDateTimeHEX() {
    return SN_Utils__U64ToHexString(SN_Utils__GetCurrentTimestamp());
}

String SN_Utils__FormatDateTimeHEX(uint64_t dateTime) {
    return SN_Utils__U64ToHexString(dateTime);
}

String SN_Utils__U8ToHEX(uint8_t value) {
    return SN_Utils__ToHex(value);
}

String SN_Utils__U16ToHEX(uint16_t value) {
    char buf[5]; // Buffer to hold the resulting hexadecimal string (4 characters + null terminator)
    sprintf(buf, "%04X", value);
    return String(buf);
}

String SN_Utils__U32ToHEX(uint32_t value) {
    char buf[9]; // Buffer to hold the resulting hexadecimal string (8 characters + null terminator)
    sprintf(buf, "%08X", value);
    return String(buf);
}

// Combine two bytes into a 32-bit value (little-endian)
uint16_t SN_Utils__U8ToU16(uint8_t lowByte, uint8_t highByte) {
    return (static_cast<uint16_t>(lowByte) << 0) |
           (static_cast<uint16_t>(highByte) << 8);
}

void SN_Utils__U16ToU8(uint16_t value, uint8_t *lowByte, uint8_t *highByte) {
    *lowByte = static_cast<uint8_t>(value & 0xFF);
    *highByte = static_cast<uint8_t>((value >> 8) & 0xFF);
}

// Combine four bytes into a 32-bit value (little-endian)
uint32_t SN_Utils__U8ToU32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    return (static_cast<uint32_t>(byte0) << 0) |
           (static_cast<uint32_t>(byte1) << 8) |
           (static_cast<uint32_t>(byte2) << 16) |
           (static_cast<uint32_t>(byte3) << 24);
}

void SN_Utils__U32ToU8(uint32_t value, uint8_t *byte0, uint8_t *byte1, uint8_t *byte2, uint8_t *byte3) {
    *byte0 = static_cast<uint8_t>(value & 0xFF);
    *byte1 = static_cast<uint8_t>((value >> 8) & 0xFF);
    *byte2 = static_cast<uint8_t>((value >> 16) & 0xFF);
    *byte3 = static_cast<uint8_t>((value >> 24) & 0xFF);
}

String SN_Utils__U8ToU32Hex(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    char buf[9]; // Buffer to hold the resulting hexadecimal string (8 characters + null terminator)
    sprintf(buf, "%02X%02X%02X%02X", byte3, byte2, byte1, byte0);
    return String(buf);
}

String SN_Utils__FormatFirmwareBuildDateTime() {
    DateTime dateTime(SN_Utils__GetBuildTimeStamp());

    return formatDate(dateTime);
}

String SN_Utils__LastUpdateDateTime() {
    if (lastDataTimeStampFromBroker_utc == 0)
        return String("00.00.0000 00:00:00");

    DateTime dateTime(static_cast<uint32_t>(lastDataTimeStampFromBroker_utc / 1000ULL));

    return formatDate(dateTime);
}

String SN_Utils__Format_DateTime(uint32_t timestamp) {
    DateTime dateTime(timestamp);

    return formatDate(dateTime);
}

// Format [DD.MM.YYYY]
String SN_Utils__Format_DateTimeShort(uint32_t timestamp) {
    if (timestamp == 0)
        return String("00.00.0000");

    DateTime dateTime(timestamp);

    char buf[64];

    sprintf(buf, "%02d.%02d.%04d", dateTime.day(), dateTime.month(), dateTime.year());

    return String(buf);


}

bool SN_Utils__IsTimeWasSyncedWithServer() {
    return lastDataTimeStampFromBroker_utc > 0;
}

String SN_Utils__GetUniqueID() {
    String mac = WiFi.macAddress();
    mac.toUpperCase();
    mac.replace(":", "");

    return mac;
}

bool SN_Utils__IsEmpty(String str) {
    if (str == nullptr)
        return true;

    str.trim();

    return str.length() == 0;
}

bool SN_Utils__IsNotEmpty(String str) {
    return !SN_Utils__IsEmpty(str);
}

String SN_Utils__URLEncode(String value) {
    return urlencode(value);
}

bool SN_Utils__IsCinderellaTime() {
    DateTime dateTime(static_cast<uint32_t>(SN_Utils__GetNow()));

    return (dateTime.hour() == 0) && (dateTime.minute() == 0) &&
           ((dateTime.second() == 0) || (dateTime.second() == 1) || (dateTime.second() == 2));
}

// Format [DD.MM.YYYY hh:mm:ss]
String formatDate(DateTime dateTime) {
    char buf[64];

    sprintf(buf, "%02d.%02d.%04d %02d:%02d:%02d", dateTime.day(), dateTime.month(), dateTime.year(), dateTime.hour(),
            dateTime.minute(), dateTime.second());

    return String(buf);
}

// Format [DD/MM/YYYY]
String formatDateShort(DateTime dateTime) {
    char buf[16];  // Increased buffer size to 16 to ensure sufficient space (DD/MM/YYYY = 10 chars + null = 11, extra buffer for safety)

    sprintf(buf, "%02d/%02d/%04d", dateTime.day(), dateTime.month(), dateTime.year());

    return String(buf);
}

// Format [YYYYMMDDhhmm]
String formatDateVersion(DateTime dateTime) {
    char buf[64];

    sprintf(buf, "%04d%02d%02d%02d%02d", dateTime.year(), dateTime.month(), dateTime.day(), dateTime.hour(),
            dateTime.minute());

    return String(buf);
}


String urldecode(const String &str) {
    String encodedString;
    encodedString.reserve(str.length()); // Pre-allocate memory

    for (int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c == '+') {
            encodedString += ' ';
        } else if (c == '%') {
            if (i + 2 < str.length()) {
                char code0 = str.charAt(++i);
                char code1 = str.charAt(++i);
                encodedString += static_cast<char>((h2int(code0) << 4) | h2int(code1));
            }
        } else {
            encodedString += c;
        }
        yield();
    }

    return encodedString;
}

String urlencode(const String &str) {
    String encodedString;
    encodedString.reserve(str.length() * 3); // Each char may expand to up to 3 chars

    for (int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c == ' ') {
            encodedString += '+';
        } else if (isalnum(c)) {
            encodedString += c;
        } else {
            encodedString += '%';
            encodedString += String("0123456789ABCDEF")[(c >> 4) & 0xF];
            encodedString += String("0123456789ABCDEF")[c & 0xF];
        }
        yield();
    }

    return encodedString;
}

unsigned char h2int(char c) {
    if (c >= '0' && c <= '9') {
        return ((unsigned char) c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return ((unsigned char) c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return ((unsigned char) c - 'A' + 10);
    }
    return (0);
}
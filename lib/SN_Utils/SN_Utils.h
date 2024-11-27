#include <Arduino.h>

long SN_Utils__GetNow();

String SN_Utils__FormatBytes(size_t bytes);

uint64_t SN_Utils__Millis64();

void SN_Utils__UpdateSystemMills();

uint64_t SN_Utils__GetCurrentTimestamp();

String SN_Utils__FormatCurrentDate();

String SN_Utils__FormatCurrentDateTime();

String SN_Utils__FormatDateTime(uint32_t value);

String SN_Utils__FormatCurrentDateTimeHEX();

String SN_Utils__TimestampToStr(uint64_t timestamp);

String SN_Utils__FormatDateTimeHEX(uint64_t dateTime);

String SN_Utils__LastUpdateDateTime();

String SN_Utils__Format_DateTime(uint32_t timestamp);

String SN_Utils__Format_DateTimeShort(uint32_t timestamp);

bool SN_Utils__IsTimeWasSyncedWithServer();

bool SN_Utils__IsCinderellaTime();

String SN_Utils__GetUniqueID();

bool SN_Utils__IsEmpty(String str);

bool SN_Utils__IsNotEmpty(String str);

String SN_Utils__URLEncode(String value);

String SN_Utils__U8ToHEX(uint8_t value);

String SN_Utils__U16ToHEX(uint16_t value);

String SN_Utils__U32ToHEX(uint32_t value);

String SN_Utils__FormatFirmwareBuildDateTime();

String SN_Utils__FormatFirmwareVersion();

String SN_Utils__Get_HEX_Values_WithCheckNull(byte *array, int len);

String SN_Utils__StringToHex(String value);

uint64_t SN_Utils__HexStringToU64(String str);

String SN_Utils__HexStringToDecString(String str);

void SN_Utils__HexStringToByteArray(String value, byte *array);

String SN_Utils__U64ToHexString(uint64_t value);

String SN_Utils__U64ToDecString(uint64_t value);

String SN_Utils__ToHex(byte value);

String SN_Utils__ByteArrayToFormatString(byte array[], unsigned int len);

String SN_Utils__ByteArrayToHexString(byte array[], unsigned int len);

String SN_Utils__ByteArrayToHexString(byte array[], int16_t start_index, unsigned int len);

String SN_Utils__ByteArrayToString(byte array[], unsigned int len);

String SN_Utils__ByteArrayToString(byte array[], uint16_t start_index, uint16_t len);

String SN_Utils__ByteArrayToString_Format(byte array[], uint16_t start_index, uint16_t len);

String SN_Utils__DecByteArrayToString(byte array[], unsigned int len);

String SN_Utils__DecByteArrayToString(byte array[], int16_t start_index, unsigned int len);

void SN_Utils__SetCurrentTime(uint64_t timestamp);

unsigned int SN_Utils__GetBuildTimeStamp();

uint16_t SN_Utils__U8ToU16(uint8_t lowByte, uint8_t highByte);

void SN_Utils__U16ToU8(uint16_t value, uint8_t *lowByte, uint8_t *highByte);

uint32_t SN_Utils__U8ToU32(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);

void SN_Utils__U32ToU8(uint32_t value, uint8_t *byte0, uint8_t *byte1, uint8_t *byte2, uint8_t *byte3);

String SN_Utils__U8ToU32Hex(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);


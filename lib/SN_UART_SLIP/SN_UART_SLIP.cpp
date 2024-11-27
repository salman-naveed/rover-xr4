#include <Arduino.h>
#include <SN_UART_SLIP.h>
#include <SN_Logger.h>


void SN_UART_SLIP_Init() {
    Serial.setRxBufferSize(4096);

    Serial.begin(115200, SERIAL_8N1);

    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    logMessage(true, "SN_UART_SLIP_Init", "Serial port initialized");
}
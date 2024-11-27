
#define DEFAULT_UART_BAUDRATE 115200

#define SLIP_BAUDRATE 115200

#define SLIP_RX_BUFFER_SIZE 4096 // 1024

#define SLIP_COMMUNICATION_TYPE 0   // 0 - SLIP will use default Serial object and the default communication settings (usually SERIAL_8N1)
                                    // 1 - SLIP will use Non-Standard/user-configured Serial Configuration Serial communication using Stream interface
                                    // 2 - SLIP will use Secondary Serial Ports (e.g. Serial1, Serial2, etc)
                                    // 3 - SLIP will use SoftwareSerial (uncomment the SoftwareSerial object declaration below)


#define SLIP_SOFTWARE_SERIAL_PIN_1 10
#define SLIP_SOFTWARE_SERIAL_PIN_2 11


void SN_UART_SLIP_Init();

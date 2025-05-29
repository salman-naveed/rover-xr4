#include <Arduino.h>
#include <SN_UART_SLIP.h>
#include <SN_Logger.h>

#include "driver/uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"


void SN_UART_SLIP_Init() {
    Serial.setRxBufferSize(SLIP_RX_BUFFER_SIZE);

    Serial.begin(DEFAULT_UART_BAUDRATE, SERIAL_8N1);

    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    logMessage(true, "SN_UART_SLIP_Init", "Serial port initialized");
}


#define UART_NUM UART_NUM_0
#define MAX_COMMANDS 16
#define INPUT_BUFFER_SIZE 128
#define MAX_ARGS 8

static command_entry_t command_table[MAX_COMMANDS];
static int command_count = 0;

void serial_console_register_command(const char *name, command_handler_t handler, const char *help) {
    if (command_count < MAX_COMMANDS) {
        command_table[command_count++] = (command_entry_t){name, handler, help};
    }
}

static void parse_and_execute(char *line) {
    char *argv[MAX_ARGS];
    int argc = 0;

    char *token = strtok(line, " \r\n");
    while (token && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " \r\n");
    }
    if (argc == 0) return;

    for (int i = 0; i < command_count; i++) {
        if (strcmp(argv[0], command_table[i].name) == 0) {
            command_table[i].handler(argc, argv);
            return;
        }
    }

    printf("Unknown command. Type 'help' for available commands.\n");
}

static void command_help(int argc, char **argv) {
    printf("Available commands:\n");
    for (int i = 0; i < command_count; i++) {
        printf("  %-10s - %s\n", command_table[i].name, command_table[i].help);
    }
}

static void serial_console_task(void *arg) {
    char input_line[INPUT_BUFFER_SIZE] = {0};
    int pos = 0;

    printf("Serial Console Ready. Type 'help' for commands.\n");

    while (true) {
        uint8_t ch;
        int len = uart_read_bytes(UART_NUM, &ch, 1, pdMS_TO_TICKS(10));
        if (len > 0) {
            if (ch == '\r' || ch == '\n') {
                if (pos > 0) {
                    input_line[pos] = '\0';
                    parse_and_execute(input_line);
                    pos = 0;
                }
            } else if (pos < INPUT_BUFFER_SIZE - 1) {
                input_line[pos++] = ch;
            }
        }
    }
}

void serial_console_start(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0);

    serial_console_register_command("help", command_help, "Show this help message");
    xTaskCreate(serial_console_task, "serial_console", 4096, NULL, 5, NULL);
}
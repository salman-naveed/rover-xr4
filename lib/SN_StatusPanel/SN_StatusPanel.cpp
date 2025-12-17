// SN_StatusPanel_re-engineered.cpp
#include <SN_StatusPanel.h>
#include <SN_XR_Board_Types.h>
#include <SN_Logger.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define BUTTON_PIN 4
#define LED_CONTROL_PIN 2
#define STRIP_LED_COUNT 8

// Define number of strips based on board type
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
#define NUM_STRIPS 2
#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32
#define NUM_STRIPS 1
#else
#error "Unsupported board type"
#endif

#define TOTAL_LEDS (STRIP_LED_COUNT * NUM_STRIPS)

Adafruit_NeoPixel leds(TOTAL_LEDS, LED_CONTROL_PIN, NEO_GRB + NEO_KHZ800);

// FreeRTOS handles
static TaskHandle_t led_task_handle = NULL;
static SemaphoreHandle_t leds_mutex = NULL;

// Define common colors
#define COLOR_BLUE leds.Color(0, 0, 255)
#define COLOR_GREEN leds.Color(0, 255, 0)
#define COLOR_RED leds.Color(255, 0, 0)
#define COLOR_YELLOW leds.Color(255, 255, 0)
#define COLOR_BLACK leds.Color(0, 0, 0)
#define COLOR_XR4 leds.Color(0, 153, 153)

volatile LED_State status_led_state = No_Color;

// Function to set status LED color and animation
void SN_StatusPanel__SetStatusLedState(LED_State state) {
  status_led_state = state;
}

void SN_StatusPanel__ControlHeadlights(bool on) {
#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32
  if (NUM_STRIPS <= 1) return; // No headlight strip on this board

  uint32_t headlightColor = on ? leds.Color(255, 255, 255) : COLOR_BLACK;

  if (xSemaphoreTake(leds_mutex, portMAX_DELAY) == pdTRUE) {
    // Apply color to the second strip (headlights)
    for (int i = STRIP_LED_COUNT; i < TOTAL_LEDS; i++) {
      leds.setPixelColor(i, headlightColor);
    }
    leds.show();
    xSemaphoreGive(leds_mutex);
  }
#endif
}

void updateStatusLEDs() {
  uint32_t color = COLOR_BLACK; // Default to black

  if (xSemaphoreTake(leds_mutex, portMAX_DELAY) != pdTRUE) return;
  
  // Only clear the status LEDs (first strip), not the headlights
  for (int i = 0; i < STRIP_LED_COUNT; i++) {
    leds.setPixelColor(i, COLOR_BLACK);
  }

  switch (status_led_state) {
    case Solid_Blue:
      color = COLOR_BLUE;
      break;
    case Solid_Green:
      color = COLOR_GREEN;
      break;
    case Solid_Red:
      color = COLOR_RED;
      break;
    case Solid_Yellow:
      color = COLOR_YELLOW;
      break;
    case Solid_XR4:
      color = COLOR_XR4;
      break;
    case Blink_Blue:
      if (millis() % 1000 < 500) color = COLOR_BLUE;
      break;
    case Blink_Red:
      if (millis() % 1000 < 500) color = COLOR_RED;
      break;
    case Blink_Green:
      if (millis() % 1000 < 500) color = COLOR_GREEN;
      break;
    case Blink_Yellow:
      if (millis() % 1000 < 500) color = COLOR_YELLOW;
      break;
    case Blink_XR4:
      if (millis() % 1000 < 500) color = COLOR_XR4;
      break;
    case Moving_Back_Forth:
        {
            // Sequential animation: one LED moves left to right repeatedly
            int ledPosition = (millis() / 200) % STRIP_LED_COUNT;
            leds.setPixelColor(ledPosition, COLOR_BLUE);
            // Show and return early since we've already set the pixel
            leds.show();
            xSemaphoreGive(leds_mutex);
            return;
        }
    case No_Color:
    default:
      break;
  }

  // Apply the color to the status LEDs (first strip only)
  for (int i = 0; i < STRIP_LED_COUNT; i++) {
    leds.setPixelColor(i, color);
  }

  leds.show();
  xSemaphoreGive(leds_mutex);
}

void led_task(void *pvParameters) {
  for (;;) {
    updateStatusLEDs();
    vTaskDelay(pdMS_TO_TICKS(50)); // Refresh rate of 20Hz
  }
}

void SN_StatusPanel__Init() {
  leds_mutex = xSemaphoreCreateMutex();

  leds.begin();
  leds.setBrightness(15); // Reduced brightness (was 50, range 0-255)
  leds.clear();
  leds.show();
  Serial.println("Status Panel Initialized");

  xTaskCreatePinnedToCore(
      led_task,           /* Task function. */
      "LED_Task",         /* name of task. */
      2048,               /* Stack size of task */
      NULL,               /* parameter of the task */
      1,                  /* priority of the task */
      &led_task_handle,   /* Task handle to keep track of created task */
      0);                 /* pin task to core 0 */
}

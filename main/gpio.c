#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_err.h"

#define LED_1 2
#define BUTTON_1 15

#define TAG "GPIO_MODULE"

xQueueHandle interruption_queue;

static void IRAM_ATTR gpio_isr_handler(void *args) {
  int pin = (int)args;
  xQueueSendFromISR(interruption_queue, &pin, NULL);
}

void handle_button_interruption(void *params) {
  int pin;
  int contador = 0;

  while(true) {
    if(xQueueReceive(interruption_queue, &pin, portMAX_DELAY)) {
      // De-bouncing
      int state = gpio_get_level(pin);
      if(state == 1) {
        gpio_isr_handler_remove(pin);
        while(gpio_get_level(pin) == state) {
          vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        contador++;
        printf("Os botões foram acionados %d vezes. Botão: %d\n", contador, pin);

        // Habilitar novamente a interrupção
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_isr_handler_add(pin, gpio_isr_handler, (void *) pin);
      }

    }
  }
}

void init_button() {
  ESP_LOGI(TAG, "Initializing BUTTON");

  // Configuração dos pinos dos LEDs 
  gpio_pad_select_gpio(LED_1);   
  // Configura os pinos dos LEDs como Output
  gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);  

  // Configuração do pino do Botão
  gpio_pad_select_gpio(BUTTON_1);
  // Configura o pino do Botão como Entrada
  gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
  // Configura o resistor de Pulldown para o botão (por padrão a entrada estará em Zero)
  gpio_pulldown_en(BUTTON_1);
  // Desabilita o resistor de Pull-up por segurança.
  gpio_pullup_dis(BUTTON_1);

  // Configura pino para interrupção
  gpio_set_intr_type(BUTTON_1, GPIO_INTR_POSEDGE);

  interruption_queue = xQueueCreate(10, sizeof(int));
  xTaskCreate(handle_button_interruption, "handle_button_interruption", 2048, NULL, 1, NULL);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(BUTTON_1, gpio_isr_handler, (void *) BUTTON_1);
}
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "esp_log.h"
#include "esp_err.h"
#include "cJSON.h"
#include "mqtt.h"
#include "global.h"

#define BUTTON_1 0

#define IS_TOUCH CONFIG_ESP_TOUCH

#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_THRESH_PERCENT  (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

#define TAG "GPIO_MODULE"

xQueueHandle interruption_queue;

void send_message (int value) {
    cJSON *root = cJSON_CreateObject();
    // cJSON add bool value
    ESP_LOGI(TAG, "Sending message state: %d\n", value);
    cJSON_AddBoolToObject(root, "value", value == 1 ? true : false);
    // send message base topic + "/estado"
    char * topic = malloc(g_base_topic_len + strlen("estado") + 1);
    if (g_base_topic == NULL) {
      ESP_LOGE(TAG, "Base topic is NULL, ESP may not registered");
      return;
    }
    strcpy(topic, g_base_topic);
    strcat(topic, "estado");
    mqtt_envia_mensagem(topic, cJSON_PrintUnformatted(root));
    cJSON_Delete(root);
    free(topic);
}

#if IS_TOUCH

static uint32_t s_pad_init_val[TOUCH_PAD_MAX];

static uint32_t last_time_pressed = 0;

typedef struct {
    int pad;
    int value;
} t_pad;

/*
  Read values sensed at all available touch pads.
  Use 2 / 3 of read value as the threshold
  to trigger interrupt when the pad is touched.
  Note: this routine demonstrates a simple way
  to configure activation threshold for the touch pads.
  Do not touch any pads when this routine
  is running (on application start).
 */
static void tp_example_set_thresholds(void)
{
    uint16_t touch_value;
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        //read filtered value
        touch_pad_read_filtered(i, &touch_value);
        s_pad_init_val[i] = touch_value;
        ESP_LOGI(TAG, "test init: touch pad [%d] val is %d", i, touch_value);
        //set interrupt threshold.
        ESP_ERROR_CHECK(touch_pad_set_thresh(i, touch_value * 2 / 3));

    }
}

/*
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
 */
static void tp_example_rtc_intr(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    //clear interrupt
    touch_pad_clear_status();
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        if ((pad_intr >> i) & 0x01) {
            t_pad pad = {
                .pad = i,
                .value = true,
            };
            xQueueSendFromISR(interruption_queue, &pad, NULL);
        }
    }
}

/*
 * Before reading touch pad, we need to initialize the RTC IO.
 */
static void tp_example_touch_pad_init(void)
{
    for (int i = 0; i < TOUCH_PAD_MAX; i++) {
        //init RTC IO and mode for touch pad.
        touch_pad_config(i, TOUCH_THRESH_NO_USE);
    }
}

void handle_touch_interruption(void *arg){
    t_pad pad;
    while (1) {
        if (xQueueReceive(interruption_queue, &pad, portMAX_DELAY)) {
            // update last time pressed
            last_time_pressed = esp_log_timestamp();
        }
    }
}

void task_value_observer(void *arg) {
    int last_value = 0;
    while (1) {
        // if last time pressed is greater than 200ms 
        if (esp_log_timestamp() - last_time_pressed > 200) {
            send_message(0);
        } else {
            if (last_value != 1) {
                send_message(1);
                last_value = 1;
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}

void init_button(void)
{
    // Initialize touch pad peripheral, it will start a timer to run a filter
    ESP_LOGI(TAG, "Initializing touch pad");
    ESP_ERROR_CHECK(touch_pad_init());
    // If use interrupt trigger mode, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    // Set reference voltage for charging/discharging
    // For most usage scenarios, we recommend using the following combination:
    // the high reference valtage will be 2.7V - 1V = 1.7V, The low reference voltage will be 0.5V.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    // Init touch pad IO
    tp_example_touch_pad_init();
    // Initialize and start a software filter to detect slight change of capacitance.
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // Set thresh hold
    tp_example_set_thresholds();
    // Register touch interrupt ISR
    touch_pad_isr_register(tp_example_rtc_intr, NULL);
    // Start queue
    interruption_queue = xQueueCreate(10, sizeof(t_pad));
    // Enable touch pad interrupt
    touch_pad_intr_enable();
    // Start a task to show what pads have been touched
    xTaskCreate(handle_touch_interruption, "touch pad task", 2048, NULL, 5, NULL);
    xTaskCreate(task_value_observer, "task_value_observer", 2048, NULL, 5, NULL);
}

#else // Button MODE

static void IRAM_ATTR gpio_isr_handler(void *args) {
  int pin = (int)args;
  xQueueSendFromISR(interruption_queue, &pin, NULL);
}

void handle_button_interruption(void *params) {
  int pin;

  while(true) {
    if(xQueueReceive(interruption_queue, &pin, portMAX_DELAY)) {
      // De-bouncing
      int state = gpio_get_level(pin);
      int count = 0;    
      gpio_isr_handler_remove(pin);

      for (int i = 0; i < 10; i++) {
        count += gpio_get_level(pin) == state ? 1 : 0;
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      if (count > 8) {
        send_message(state);
      }

      // Habilitar novamente a interrupção
      vTaskDelay(50 / portTICK_PERIOD_MS);
      gpio_isr_handler_add(pin, gpio_isr_handler, (void *) pin);
      

    }
  }
}

void init_button() {
  ESP_LOGI(TAG, "Initializing BUTTON");

  gpio_pad_select_gpio(BUTTON_1);
  // Configura o pino do Botão como Entrada
  gpio_set_direction(BUTTON_1, GPIO_MODE_INPUT);
  // Configura o resistor de Pulldown para o botão (por padrão a entrada estará em Zero)
  gpio_pulldown_en(BUTTON_1);
  // Desabilita o resistor de Pull-up por segurança.
  gpio_pullup_dis(BUTTON_1);

  // Configura pino para interrupção
  gpio_set_intr_type(BUTTON_1, GPIO_INTR_ANYEDGE);

  interruption_queue = xQueueCreate(10, sizeof(int));
  xTaskCreate(handle_button_interruption, "handle_button_interruption", 2048, NULL, 1, NULL);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(BUTTON_1, gpio_isr_handler, (void *) BUTTON_1);
}

#endif // IS_TOUCH
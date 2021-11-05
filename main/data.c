#include "data.h"

#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht11.h"
#include "cJSON.h"
#include "mqtt.h"
#include "global.h"

#include "esp_log.h"

#define TEMPERATURE_LENGTH 11
#define HUMIDITY_LENGHT 8
#define TAG "DHT11"

void ler_sensor(void * params) {
  DHT11_init(GPIO_NUM_4);

  // mount temp and humi jsons
  cJSON *temp_json = cJSON_CreateObject();
  cJSON *humi_json = cJSON_CreateObject();
  // format {value: xxx}
  cJSON_AddNumberToObject(temp_json, "value", 0);
  cJSON_AddNumberToObject(humi_json, "value", 0);


  // TODO - reflect base topic changes
  char *temp_topic = malloc(g_base_topic_len + TEMPERATURE_LENGTH + 1 );
  char *humi_topic = malloc(g_base_topic_len + HUMIDITY_LENGHT + 1 );
  temp_topic = strcpy(temp_topic, g_base_topic);
  humi_topic = strcpy(humi_topic, g_base_topic);
  temp_topic = strcat(temp_topic, "temperatura");
  humi_topic = strcat(humi_topic, "umidade");
  temp_topic[g_base_topic_len + TEMPERATURE_LENGTH] = '\0';
  humi_topic[g_base_topic_len + HUMIDITY_LENGHT] = '\0';

  while(1) {
    float temp = 0.0f;
    float hum = 0.0f;
    int count = 0;
    for (int i = 0; i < 5; i++) {
      struct dht11_reading ret = DHT11_read(&temp, &hum);

      if (ret.status == DHT11_OK) {
        ESP_LOGI(TAG, "Read from sensor status: OK");
        ESP_LOGI(TAG, "Temperature: %d C", ret.temperature);
        ESP_LOGI(TAG, "Humidity: %d %%", ret.humidity);
        count++;
        temp += ret.temperature;
        hum += ret.humidity;
      }else {
        ESP_LOGI(TAG, "Read from sensor status: ERROR");
      }
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    temp /= count;
    hum /= count;

    // update temp and humi jsons
    cJSON_ReplaceItemInObject(temp_json, "value", cJSON_CreateNumber(temp));
    cJSON_ReplaceItemInObject(humi_json, "value", cJSON_CreateNumber(hum));

    // send to mqtt
    mqtt_envia_mensagem(temp_topic, cJSON_PrintUnformatted(temp_json));
    mqtt_envia_mensagem(humi_topic, cJSON_PrintUnformatted(humi_json));
  }
}
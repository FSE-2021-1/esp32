#include "init.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "data.h"
#include "mqtt.h"

#define TAG "init"

#define MATR CONFIG_ESP_MATRICULA

void init_topics() {
    char *prefix = "fse2021/" MATR "/";
    int base_topic_len = strlen(prefix) + g_local_len;
    char *base_topic = malloc(base_topic_len + 2);
    strcpy(base_topic, prefix);
    strcat(base_topic, g_local);
    base_topic[base_topic_len] = '/';
    base_topic[base_topic_len+1] = '\0';
    g_base_topic = base_topic;
    g_base_topic_len = base_topic_len+1;
    ESP_LOGI(TAG, "Registered!! Base topic: %s", base_topic);

    // Subscribe to base topic + 'estado'
    char *topic = malloc(base_topic_len + strlen("estado") + 1);
    strcpy(topic, base_topic);
    strcat(topic, "estado");
    ESP_LOGI(TAG, "Subscribing to %s", topic);
    mqtt_topic_subscribe(topic);
    free(topic);

    xTaskCreate(&ler_sensor, "Leitura DHT", 4096, NULL, 2, g_dht_task_handle);
}
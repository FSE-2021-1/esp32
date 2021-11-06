#include "mqtt_router.h"
#include "wifi.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "data.h"
#include "mqtt.h"
#include "pwm.h"

#define MATR CONFIG_ESP_MATRICULA

#define TAG "MQTT_ROUTER"

void route_mqtt_register(int payload_len, char *payload) {
    ESP_LOGI(TAG, "Registering...\n");
    cJSON *root = cJSON_Parse(payload);
    cJSON *local = cJSON_GetObjectItemCaseSensitive(root, "local");
    if (local == NULL) {
        ESP_LOGE(TAG, "Error: local not found\n");
        cJSON_Delete(root);
        return;
    }
    int local_len = strlen(local->valuestring);
    char *local_str = malloc(local_len);
    strcpy(local_str, local->valuestring);
    ESP_LOGI(TAG, "Local: %s", local_str);
    g_local = local_str;
    g_local_len = local_len;
    
    char *prefix = "fse2021/" MATR "/";
    int base_topic_len = strlen(prefix) + local_len;
    char *base_topic = malloc(base_topic_len + 2);
    strcpy(base_topic, prefix);
    strcat(base_topic, local_str);
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

    if(g_dht_task_handle != NULL) {
        vTaskDelete(g_dht_task_handle);
    }

    xTaskCreate(&ler_sensor, "Leitura DHT", 4096, NULL, 2, &g_dht_task_handle); 
}

void route_mqtt_state(int topic_len, char *topic, int payload_len, char *payload) {
    // get state from payload json
    cJSON *root = cJSON_Parse(payload);
    cJSON *state = cJSON_GetObjectItemCaseSensitive(root, "out");
    if (state == NULL) {
        ESP_LOGI(TAG, "output state not found\n");
        cJSON_Delete(root);
        return;
    }
    int state_int = state->valueint;
    set_pwm_value(state_int > 255 ? 255 : (state_int < 0 ? 0 : state_int));
}

void mqtt_router_route(int topic_len, char *topic, int payload_len, char *payload) {
    // check if topic match with fse2021/<matricula>/dispositivos/<mac_address>
    char * mac_address = get_mac_address();
    if (topic_len == strlen("fse2021") + strlen(MATR) + strlen("/dispositivos/") + strlen(mac_address) + 1) {
        if (strncmp(topic, "fse2021/" MATR "/dispositivos/", strlen("fse2021/" MATR "/dispositivos/")) == 0) {
            if (strncmp(topic + strlen("fse2021/" MATR "/dispositivos/"), mac_address, strlen(mac_address)) == 0) {
                route_mqtt_register(payload_len, payload);
            }
        }
    }

    // check if topic match with fse2021/<matricula>/<local>/estado
    if (topic_len == strlen("fse2021") + strlen(MATR) + g_local_len + strlen("/estado") + 1) {
        if (strncmp(topic, "fse2021/" MATR "/", strlen("fse2021/" MATR "/")) == 0) {
            if (strncmp(topic + strlen("fse2021/" MATR "/"), g_local, g_local_len) == 0) {
                if (strncmp(topic + strlen("fse2021/" MATR "/") + g_local_len, "/estado", strlen("/estado")) == 0) {
                    route_mqtt_state(topic_len, topic, payload_len, payload);
                }
            }
        }
    }
}
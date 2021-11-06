#include <stdio.h>
#include <string.h>
#include "register.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"
#include "mqtt.h"
#include "wifi.h"

#include "init.h"
#include "global.h"

#define TAG "Register"

#define MATR CONFIG_ESP_MATRICULA

void register_device(void * params) {
    char * mac;
    // TODO check if register is already done
    mac = get_mac_address();

    ESP_LOGI(TAG, "MAC: %s", mac);

    char * local = NULL;
    size_t local_len;
    esp_err_t err = nvs_get_str(g_nvs, "local", NULL, &local_len);
    local = malloc(local_len);
    err = nvs_get_str(g_nvs, "local", local, &local_len);
    switch (err) {
        case ESP_OK:
            ESP_LOGI(TAG, "ESP is already registered!");
            g_local = malloc(local_len);
            memcpy(g_local, local, local_len);
            // FIX 0 terminated str
            g_local_len = (int) local_len - 1;

            init_topics();
            return;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("ESP is not registered yet!\n");
            break;
        default :
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }

    char topic[100];
    sprintf(topic, "fse2021/%s/dispositivos/%s", MATR, mac);
    char payload[100];
    sprintf(payload, "{\"id\": \"%s\"}", mac);

    ESP_LOGI(TAG, "Send message to topic: %s", topic);
    ESP_LOGI(TAG, "Payload: %s", payload);
    mqtt_envia_mensagem(topic, payload);
    mqtt_topic_subscribe(topic);
}
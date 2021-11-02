#include <stdio.h>
#include "register.h"
#include "esp_log.h"
#include "mqtt.h"
#include "wifi.h"

#define TAG "Register"

#define MATR CONFIG_ESP_MATRICULA

void register_device() {
    char * mac;
    // TODO check if register is already done
    mac = get_mac_address();

    ESP_LOGI(TAG, "MAC: %s", mac);

    char topic[100];
    sprintf(topic, "fse2021/%s/dispositivos/%s", MATR, mac);
    char payload[100];
    sprintf(payload, "{\"id\": \"%s\"}", mac);

    ESP_LOGI(TAG, "Send message to topic: %s", topic);
    ESP_LOGI(TAG, "Payload: %s", payload);
    mqtt_envia_mensagem(topic, payload);
    mqtt_subscribe(topic);
}
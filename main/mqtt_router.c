#include "mqtt_router.h"
#include "wifi.h"
#include <stdio.h>
#include <string.h>

#define MATR CONFIG_ESP_MATRICULA

void route_mqtt_register(int payload_len, char *payload) {
    printf("Registering...\n");
    printf("Payload: %s\n", payload);
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
}
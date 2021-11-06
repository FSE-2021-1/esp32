#include "global.h"
#include <stdlib.h>
#include "esp_log.h"

#define TAG "global"

char * g_local = NULL;
int g_local_len = 0;

char * g_base_topic = NULL;
int g_base_topic_len = 0;

TaskHandle_t * g_dht_task_handle = NULL;
nvs_handle_t g_nvs = 0;

void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    err = nvs_open("storage", NVS_READWRITE, &g_nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

}

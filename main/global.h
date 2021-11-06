#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

extern char * g_local;
extern char * g_base_topic;
extern int g_local_len;
extern int g_base_topic_len;
extern nvs_handle_t g_nvs;

void init_nvs();

extern TaskHandle_t * g_dht_task_handle;

#endif
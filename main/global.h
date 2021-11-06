#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern char * g_local;
extern char * g_base_topic;
extern int g_local_len;
extern int g_base_topic_len;

extern task_handle_t g_dht_task_handle;

#endif
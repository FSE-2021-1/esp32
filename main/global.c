#include "global.h"
#include <stdlib.h>

char * g_local = NULL;
int g_local_len = 0;

char * g_base_topic = NULL;
int g_base_topic_len = 0;

task_handle_t g_dht_task_handle = NULL;
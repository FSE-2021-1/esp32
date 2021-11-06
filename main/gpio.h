#ifndef __GPIO_H__
#define __GPIO_H__

void init_button();

void handle_button_interruption(void *params);

static void IRAM_ATTR gpio_isr_handler(void *args);

#endif
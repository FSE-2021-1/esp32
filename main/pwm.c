#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_err.h"
#include "pwm.h"

#define LED_1 2
#define TAG "PWM_MODULE"

int duty = 0;

void update_duty(void *params) {
    int last_duty = 0;
    while (1) {
        if (duty != last_duty) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            last_duty = duty;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void init_pwm() {
    ESP_LOGI(TAG, "Initializing PWM");
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t timer_e = ledc_timer_config(&timer_config);
    ESP_LOGI(TAG, "ledc_timer_config returned: %d", timer_e);
    if (timer_e != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed\n");
    }

    ledc_channel_config_t channel_config = {
        .gpio_num = LED_1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    esp_err_t channel_e = ledc_channel_config(&channel_config);
    ESP_LOGI(TAG, "ledc_channel_config returned: %d", channel_e);
    if (channel_e != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed\n");
    }

    ESP_LOGI(TAG, "PWM was initialized successfully");

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);


    xTaskCreate(update_duty, "update_duty", 2048, NULL, 5, NULL);
}

void set_pwm_value(int value) {
    duty = value;
}

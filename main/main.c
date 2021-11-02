/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht11.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include "wifi.h"
#include "http_client.h"
#include "mqtt.h"
#include "register.h"

xSemaphoreHandle conexaoWifiSemaphore;
xSemaphoreHandle conexaoMQTTSemaphore;

void conectadoWifi(void * params)
{
  while(true)
  {
    if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
    {
      // Processamento Internet
      mqtt_start();
    }
  }
}

void ler_temperatura(void * params) {
  DHT11_init(GPIO_NUM_4);

  while(1) {
      printf("Temperature is %d \n", DHT11_read().temperature);
      printf("Humidity is %d\n", DHT11_read().humidity);
      printf("Status code is %d\n", DHT11_read().status);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
  } 
}

void trataComunicacaoComServidor(void * params)
{
  char mensagem[50];
  if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
  {
    while(true)
    {
       float temperatura = 20.0 + (float)rand()/(float)(RAND_MAX/10.0);
       sprintf(mensagem, "temperatura1: %f", temperatura);
       mqtt_envia_mensagem("sensores/temperatura", mensagem);
       vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
  }
}

void app_main(void){
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    wifi_start();
    xTaskCreate(&conectadoWifi,  "Conexão ao MQTT", 4096, NULL, 1, NULL);


    register_device();
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
    xTaskCreate(&ler_temperatura, "Leitura de Temperatura", 4096, NULL, 1, NULL);
}


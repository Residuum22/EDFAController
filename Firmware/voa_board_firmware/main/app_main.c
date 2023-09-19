#include <stdint.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "wifi_common.h"
#include "mqtt3.h"
#include "voa_control.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "led_indicator_voa.h"

static const char *TAG = "MAIN";

QueueHandle_t voa_attenuation_queue;

static inline void voa_attenuation_queue_init()
{
    // Creating an uint8_t queue with 10 elements
    voa_attenuation_queue = xQueueCreate(10, sizeof(uint8_t));
}

void app_main(void)
{
    esp_log_default_level = ESP_LOG_DEBUG;
    ESP_LOGI(TAG, "Startup...");

    ESP_LOGI(TAG, "Initialization of nvs, netif and event loop...");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Initialize LED indicator...");
    voa_indicator_init();

    ESP_LOGI(TAG, "Connect to the wifi network...");
    ESP_ERROR_CHECK(wifi_connect());

    ESP_LOGI(TAG, "Start MQTT client...");
    mqtt_app_start();

    ESP_LOGI(TAG, "Initialize VOA attenuation queue...");
    voa_attenuation_queue_init();

    ESP_LOGI(TAG, "Start VOA control task...");
    xTaskCreate(voa_control_task, "voa_control_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Initialization done.");
}
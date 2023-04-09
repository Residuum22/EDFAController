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

#include "common_gpio.h"
#include "led_indicator.h"
#include "led_indicator_blink_default.h"

static const char *TAG = "MAIN";

QueueHandle_t voa_attenuation_queue;
led_indicator_handle_t led_state_handle;
led_indicator_handle_t led_error_handle;

static void voa_attenuation_queue_init()
{
    // Creating an uint8_t queue with 10 elements
    voa_attenuation_queue = xQueueCreate(10, sizeof(uint8_t));
}

static void voa_control_task(void *pvParameters)
{

    uint8_t attenuation = 0;

    voa_control_init_fwd();
    voa_control_init_rev();

    bool value = false;

    for (;;)
    {
        xQueueReceive(voa_attenuation_queue, &attenuation, portMAX_DELAY);
        ESP_LOGI(TAG, "Attenuation: %d", attenuation);
        if (value)
        {
            voa_control_disable_rev();
            voa_control_enable_fwd();
        }
        else
        {
            voa_control_disable_fwd();
            voa_control_enable_rev();
        }
        value = !value;
    }
}

static void voa_indicator_init()
{
    led_indicator_gpio_config_t led_state_gpio_config = {
        .is_active_level_high = true,
        .gpio_num = INDICATOR_GREEN_GPIO,
    };
    led_indicator_config_t led_state_config = {
        .mode = LED_GPIO_MODE,
        .led_indicator_gpio_config = &led_state_gpio_config,
        .blink_lists = default_led_indicator_blink_lists,
        .blink_list_num = DEFAULT_BLINK_LIST_NUM,
    };
    led_state_handle = led_indicator_create(&led_state_config);


    led_indicator_gpio_config_t led_error_gpio_config = {
        .is_active_level_high = true,
        .gpio_num = INDICATOR_RED_GPIO,
    };
    led_indicator_config_t led_error_config = {
        .mode = LED_GPIO_MODE,
        .led_indicator_gpio_config = &led_error_gpio_config,
        .blink_lists = default_led_indicator_blink_lists,
        .blink_list_num = DEFAULT_BLINK_LIST_NUM,
    };
    led_error_handle = led_indicator_create(&led_error_config);
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
    led_indicator_start(led_state_handle, BLINK_CONNECTING);

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

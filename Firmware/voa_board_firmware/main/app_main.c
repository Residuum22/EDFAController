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

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "driver/gpio.h"

static const char *TAG = "MAIN";

QueueHandle_t voa_attenuation_queue;
led_indicator_handle_t led_state_handle;
led_indicator_handle_t led_error_handle;

#define POT_VOLTAGE 4095
#define MAX_VOA_ATTENUATION_IN_V POT_VOLTAGE * 0.98
#define MIN_VOA_ATTENUATION_IN_V POT_VOLTAGE * 0.4

// https://www.calculator.net/slope-calculator.html
#define SLOPE 122.85
#define CONVERT_DB_TO_VOLTAGE(x) (x * SLOPE + MIN_VOA_ATTENUATION_IN_V)

adc_cali_handle_t adc1_cali_handle = NULL;
bool do_calibration1;
adc_oneshot_unit_handle_t adc1_handle;

static void adc_init(void)
{
    //-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC2 Calibration Init---------------//
    do_calibration1 = false;

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
}

static void voa_attenuation_queue_init()
{
    // Creating an uint8_t queue with 10 elements
    voa_attenuation_queue = xQueueCreate(10, sizeof(uint8_t));
}

static void voa_set_attenuation_zero()
{
    int adc_value = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));

    while (adc_value > MIN_VOA_ATTENUATION_IN_V)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
        ESP_LOGI(TAG, "Set VOA in the default state");
        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANNEL_0, adc_value);
        voa_control_disable_fwd();
        voa_control_enable_rev();
    }
    voa_control_disable_rev();
}

static void voa_control_task(void *pvParameters)
{
    uint8_t attenuation = 0;

    voa_control_init_fwd();
    voa_control_init_rev();

    int adc_value = 0;

    // TODO: Remove this
    // Configure GPIO4 as input becasse it is connected to the VOA potentiometer by default but it is unable to use
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_4),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&io_conf);
    // End of TODO

    voa_set_attenuation_zero();

    int last_adc_value = MIN_VOA_ATTENUATION_IN_V;

    for (;;)
    {
        xQueueReceive(voa_attenuation_queue, &attenuation, portMAX_DELAY);
        ESP_LOGI(TAG, "Attenuation: %d", attenuation);
        int espected_raw = CONVERT_DB_TO_VOLTAGE(attenuation);
        if (espected_raw > MAX_VOA_ATTENUATION_IN_V)
        {
            espected_raw = MAX_VOA_ATTENUATION_IN_V;
        }
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));

        if (last_adc_value < espected_raw)
        {
            voa_control_disable_rev();
            voa_control_enable_fwd();
            while (adc_value < espected_raw)
            {
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
            }
        }
        else
        {
            voa_control_disable_fwd();
            voa_control_enable_rev();
            while (adc_value > espected_raw)
            {
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
            }
        }

        last_adc_value = adc_value;
        voa_control_disable_fwd();
        voa_control_disable_rev();
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

    ESP_LOGI(TAG, "Initialize ADC...");
    adc_init();

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

#include "common_gpio.h"
#include "laser_module_adc.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"

#include "math.h"

static const char *TAG = "LASER_ADC";

// ADC1 oneshot configuration
static bool do_calibration1;
static adc_oneshot_unit_handle_t adc1_handle;

static uint32_t convert_raw_to_temp(uint32_t value)
{   
    // Calculated with my Excel sheet and 
    // http://www.learningaboutelectronics.com/Articles/NTC-thermistor-voltage-divider-temperature-calculator.php
    // float temperature_float = value * 0.030978934324659 - 35;

    // Calculated with my Excel sheet
    float temperature_float = 5.9991 * exp(0.0007 * value);
    return (uint32_t)floor(temperature_float);
}

static uint32_t read_laser1_raw_adc_value()
{
    int temperature1;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER1_TEMP_CHANNEL, &temperature1));
    return (uint32_t)temperature1;
}

static uint32_t read_laser2_raw_adc_value()
{
    int temperature2;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER2_TEMP_CHANNEL, &temperature2));
    return (uint32_t)temperature2;
}

static void inline laser_module_adc_gpio_init()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << LASER1_TEMP_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << LASER2_TEMP_PIN);
    gpio_config(&io_conf);
}

void laser_module_adc_init()
{
    // Init gpio's for ADC
    laser_module_adc_gpio_init();

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

    // Configure temperature channels
    ESP_LOGI(TAG, "Configuring ADC for Laser Temperatures");
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LASER1_TEMP_CHANNEL, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LASER2_TEMP_CHANNEL, &config));

    // Configure monitor diode channels
    ESP_LOGI(TAG, "Configuring ADC for Laser Monitor Diode");
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LASER1_MONITOR_DIODE_CHANNEL, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LASER2_MONITOR_DIODE_CHANNEL, &config));
}

uint32_t laser_module_adc_read_temp1()
{
    uint32_t temp = convert_raw_to_temp(read_laser1_raw_adc_value());
    return temp;
}

uint32_t laser_module_adc_read_temp2()
{
    uint32_t temp = convert_raw_to_temp(read_laser2_raw_adc_value());
    return temp;
}

uint32_t laser_module_adc_read_laser1_monitor_diode()
{
    int monitor_diode_1;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER1_MONITOR_DIODE_CHANNEL, &monitor_diode_1));
    return (uint32_t)monitor_diode_1;
}

uint32_t laser_module_adc_read_laser2_monitor_diode()
{
    int monitor_diode_2;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER2_MONITOR_DIODE_CHANNEL, &monitor_diode_2));
    return (uint32_t)monitor_diode_2;
}
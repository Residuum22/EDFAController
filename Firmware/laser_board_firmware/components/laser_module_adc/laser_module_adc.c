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
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle;

static uint32_t convert_voltage_to_temp(uint32_t value)
{   
    // Calculated with my Excel sheet
    // y=826.75*ln(x) - 1412
    float temperature_float = exp((value + 1412) / 826.75);
    // float temperature_float = 826.75 * log(value) - 1412;
    return (uint32_t)floor(temperature_float);
}

static uint32_t read_laser1_temp_voltage()
{
    int raw_tmp;
    int voltage_tmp;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER1_TEMP_CHANNEL, &raw_tmp));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, raw_tmp, &voltage_tmp));
    return (uint32_t)voltage_tmp;
}

static uint32_t read_laser2_temp_voltage()
{
    int raw_tmp;
    int voltage_tmp;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER2_TEMP_CHANNEL, &raw_tmp));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, raw_tmp, &voltage_tmp));
    return (uint32_t)voltage_tmp;
}

static void inline laser_module_adc_gpio_init()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    io_conf.pin_bit_mask = (1ULL << LASER1_TEMP_PIN);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << LASER2_TEMP_PIN);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << LASER1_MONITOR_DIODE_PIN);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << LASER2_MONITOR_DIODE_PIN);
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
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_handle));

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
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
    uint32_t temp = convert_voltage_to_temp(read_laser1_temp_voltage());
    return temp;
}

uint32_t laser_module_adc_read_temp2()
{
    uint32_t temp = convert_voltage_to_temp(read_laser2_temp_voltage());
    return temp;
}

uint32_t laser_module_adc_read_laser1_monitor_diode()
{
    int monitor_diode_1_raw, monitor_diode_1_voltage;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER1_MONITOR_DIODE_CHANNEL, &monitor_diode_1_raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, monitor_diode_1_raw, &monitor_diode_1_voltage));
    return (uint32_t)monitor_diode_1_voltage;
}

uint32_t laser_module_adc_read_laser2_monitor_diode()
{
    int monitor_diode_2_raw, monitor_diode_2_voltage;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LASER2_MONITOR_DIODE_CHANNEL, &monitor_diode_2_raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, monitor_diode_2_raw, &monitor_diode_2_voltage));
    return (uint32_t)monitor_diode_2_voltage;
}
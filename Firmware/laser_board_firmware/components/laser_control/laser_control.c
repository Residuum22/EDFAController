#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "laser_module_dac.h"

#include "laser_module_adc.h"

static const char *TAG = "LASER_CONTROL";

extern QueueHandle_t laser1_enable_queue, laser2_enable_queue;
extern QueueHandle_t laser1_desired_current_queue, laser2_desired_current_queue;

void laser_control_task(void *pvParameters)
{
    uint32_t laser1_current = 0;
    uint32_t laser2_current = 0;

    uint8_t laser1_enable;
    uint8_t laser2_enable;

    for(;;)
    {
        xQueueReceive(laser1_enable_queue, &laser1_enable, pdMS_TO_TICKS(10));
        xQueueReceive(laser2_enable_queue, &laser2_enable, pdMS_TO_TICKS(10));

        xQueueReceive(laser1_desired_current_queue, &laser1_current, pdMS_TO_TICKS(10));
        xQueueReceive(laser2_desired_current_queue, &laser2_current, pdMS_TO_TICKS(10));

        uint32_t laser1_md = laser_module_adc_read_laser1_monitor_diode();
        uint32_t laser2_md = laser_module_adc_read_laser2_monitor_diode();

        ESP_LOGI(TAG, "Laser1 monitor diode: %d mV | Laser2 monitor diode: %d mV", laser1_md, laser2_md);

        if (laser1_enable)
        {
            ESP_LOGI(TAG, "Laser1 current: %d", laser1_current);
            laser_module_dac_write_laser1_current(laser1_current);
        }

        if (laser2_enable)
        {
            ESP_LOGI(TAG, "Laser2 current: %d", laser2_current);
            laser_module_dac_write_laser2_current(laser2_current);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
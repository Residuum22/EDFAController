#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "laser_module_dac.h"

extern QueueHandle_t laser1_enable_queue, laser2_enable_queue;
extern QueueHandle_t laser1_desired_current_queue, laser2_desired_current_queue;

void laser_control_task(void *pvParameters)
{
    uint32_t laser1_current;
    uint32_t laser2_current;

    bool laser1_enable;
    bool laser2_enable;

    for(;;)
    {
        xQueueReceive(laser1_enable_queue, &laser1_enable, pdMS_TO_TICKS(10));
        xQueueReceive(laser2_enable_queue, &laser2_enable, pdMS_TO_TICKS(10));

        xQueueReceive(laser1_desired_current_queue, &laser1_current, pdMS_TO_TICKS(10));
        xQueueReceive(laser2_desired_current_queue, &laser2_current, pdMS_TO_TICKS(10));

        // TODO: Control Loop insert here insted of setting
        if (laser1_enable)
        {
            laser_module_dac_write_laser1_current(laser1_current);
        }

        if (laser2_enable)
        {
            laser_module_dac_write_laser2_current(laser2_current);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
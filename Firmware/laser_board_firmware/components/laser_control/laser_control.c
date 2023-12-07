#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "laser_module_dac.h"

#include "laser_module_adc.h"

#include "pid_controller.h"

static const char *TAG = "LASER_CONTROL";

extern QueueHandle_t laser1_enable_queue, laser2_enable_queue;
extern QueueHandle_t laser1_desired_current_queue, laser2_desired_current_queue;

void laser_control_task(void *pvParameters)
{
  esp_log_level_set(TAG, ESP_LOG_INFO);
  uint32_t laser1_current = 0;
  uint32_t laser2_current = 0;

  uint8_t laser1_enable = 0;
  uint8_t laser2_enable = 0;

  uint32_t monitor_diode1_voltage = 0;
  uint32_t monitor_diode2_voltage = 0;

  uint32_t laser1_md = 0;
  uint32_t laser2_md = 0;

  uint32_t laser1_md_target = 0;
  uint32_t laser2_md_target = 0;

  uint16_t laser1_output_current;
  uint16_t laser2_output_current;

  pid_controller_t laser1_pid = {
    .Kp = 0.5,
    .Kd = 0,
    .Ki = 0.1,
    .tau = 0,
    .limitMin = 0,
    .limitMax = 255,
    .limitIntMin = -300,
    .limitIntMax = 300,
    .sampleTime = 1 // 1 second
  };

  pid_controller_t laser2_pid = {
    .Kp = 0.5,
    .Kd = 0,
    .Ki = 0.1,
    .tau = 0,
    .limitMin = 0,
    .limitMax = 255,
    .limitIntMin = -300,
    .limitIntMax = 300,
    .sampleTime = 1 // 1 second
  };

  pid_controller_init(&laser1_pid);
  pid_controller_init(&laser2_pid);

  for (;;)
  {
    xQueueReceive(laser1_enable_queue, &laser1_enable, pdMS_TO_TICKS(10));
    xQueueReceive(laser2_enable_queue, &laser2_enable, pdMS_TO_TICKS(10));

    if (xQueueReceive(laser1_desired_current_queue, &laser1_current, pdMS_TO_TICKS(10)) == pdTRUE)
    {
      laser_module_dac_write_laser1_current(laser1_current);
      vTaskDelay(100);
      laser1_md_target = laser_module_adc_read_laser1_monitor_diode();
    }

    if (xQueueReceive(laser2_desired_current_queue, &laser2_current, pdMS_TO_TICKS(10)))
    {
      laser_module_dac_write_laser2_current(laser2_current);
      vTaskDelay(100);
      laser2_md_target = laser_module_adc_read_laser2_monitor_diode();
    }

    laser1_md = laser_module_adc_read_laser1_monitor_diode();
    laser2_md = laser_module_adc_read_laser2_monitor_diode();

    ESP_LOGD(TAG, "Laser1 monitor diode: %d mV | Laser2 monitor diode: %d mV", laser1_md, laser2_md);

    if (true)
    {
      laser1_output_current = pid_controller_update(&laser1_pid, laser1_md_target, laser1_md);
      ESP_LOGD(TAG, "Laser1 current: %d", laser1_output_current);
      laser_module_dac_write_laser1_current(laser1_output_current);
    }

    if (true)
    {
      laser2_output_current = pid_controller_update(&laser1_pid, laser1_md_target, laser1_md);
      ESP_LOGD(TAG, "Laser2 current: %d", laser2_output_current);
      laser_module_dac_write_laser1_current(laser2_output_current);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
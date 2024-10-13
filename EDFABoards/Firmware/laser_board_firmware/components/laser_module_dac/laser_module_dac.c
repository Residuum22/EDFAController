#include <math.h>
#include <assert.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

#include "driver/dac_oneshot.h"
#include "esp_log.h"

#include "common_gpio.h"
#include "math.h"

dac_oneshot_handle_t laser1_dac_handle;
dac_oneshot_handle_t laser2_dac_handle;

static uint8_t convert_current_to_raw(uint32_t current)
{
#if CONFIG_LASER_MODULE_NUMBER == 1
  // 400 mA means 255 DAC value by 980 nm laser
  return (uint8_t)floor(current * 0.6375);
#else
  // 700 mA means 255 DAC value by 1480 nm laser
  return (uint8_t)floor(current * 0.36428571428571);
#endif
}

static void write_laser1_raw_dac_value(uint8_t dac_value)
{
  ESP_ERROR_CHECK(dac_oneshot_output_voltage(laser1_dac_handle, dac_value));
}

static void write_laser2_raw_dac_value(uint8_t dac_value)
{
  ESP_ERROR_CHECK(dac_oneshot_output_voltage(laser2_dac_handle, dac_value));
}

void laser_module_dac_init()
{
  /* DAC oneshot init */
  dac_oneshot_config_t chan0_cfg = {
      .chan_id = LASER1_DIODE_CURRENT_CHANNEL, // GPIO26
  };
  ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan0_cfg, &laser1_dac_handle));
  ESP_ERROR_CHECK(dac_oneshot_output_voltage(laser1_dac_handle, 0));

  dac_oneshot_config_t chan1_cfg = {
      .chan_id = LASER2_DIODE_CURRENT_CHANNEL, // GPIO25
  };
  ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan1_cfg, &laser2_dac_handle));
  ESP_ERROR_CHECK(dac_oneshot_output_voltage(laser2_dac_handle, 0));
}

void laser_module_dac_write_laser1_current(uint32_t current)
{
  write_laser1_raw_dac_value(convert_current_to_raw(current));
}

void laser_module_dac_write_laser2_current(uint32_t current)
{
  write_laser2_raw_dac_value(convert_current_to_raw(current));
}
#pragma once

#include <stdint.h>

void laser_module_adc_init();

uint32_t laser_module_adc_read_temp1();

uint32_t laser_module_adc_read_temp2();

uint32_t laser_module_adc_read_laser1_monitor_diode();

uint32_t laser_module_adc_read_laser2_monitor_diode();
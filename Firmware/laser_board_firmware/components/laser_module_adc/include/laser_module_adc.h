#pragma once

#include <stdint.h>

/**
 * @brief Initialize the ADC for temperature and monitor diode.
 * 
 */
void laser_module_adc_init();

/**
 * @brief Read Laser 1 temperature.
 * 
 *
 * @return Laser1 temperature
 */
uint32_t laser_module_adc_read_temp1();

/**
 * @brief Read Laser 2 temperature.
 * 
 *
 * @return Laser1 temperature
 */
uint32_t laser_module_adc_read_temp2();

/**
 * @brief Read adc value of the monitor diode.
 * 
 *
 * @return Raw ADC value
 */
uint32_t laser_module_adc_read_laser1_monitor_diode();

/**
 * @brief Read adc value of the monitor diode.
 * 
 *
 * @return Raw ADC value
 */
uint32_t laser_module_adc_read_laser2_monitor_diode();
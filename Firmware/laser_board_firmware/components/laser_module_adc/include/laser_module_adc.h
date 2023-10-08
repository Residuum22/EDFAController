#pragma once

#include <stdint.h>

/**
 * @brief Initialize the ADC for temperature and monitor diode.
 * 
 */
void laser_module_adc_init();

/**
 * @brief Read adc value of the laser1 temperature.
 * 
 *
 * @return Raw ADC value
 */
uint32_t laser_module_adc_read_temp1();

/**
 * @brief Read adc value of the laser2 temperature.
 * 
 *
 * @return Raw ADC value
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
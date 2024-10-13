#pragma once
#include <stdint.h>

/**
 * @brief Initialize DAC for Laser1/2 output
 * 
 */
void laser_module_dac_init();

/**
 * @brief Set DAC value of the Laser1
 * 
 *
 * @param[in] dac_value Raw DAC value
 */
void laser_module_dac_write_laser1_current(uint32_t current);

/**
 * @brief Set DAC value of the Laser2
 * 
 *
 * @param[in] dac_value Raw DAC value
 */
void laser_module_dac_write_laser2_current(uint32_t current);
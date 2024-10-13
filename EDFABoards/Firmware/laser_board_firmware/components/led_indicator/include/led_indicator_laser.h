#pragma once

/**
 * @brief This function initialize EDFA idicator green and red LED.
 * 
 */
void laser_indicator_init();

/**
 * @brief Set green led state
 * 
 * @param blink_type Blink type
 */
void laser_indicator_set_state(int blink_type);

/**
 * @brief Set green led state
 * 
 * @param blink_type Blink type
 */
void laser_indicator_set_error(int blink_type);
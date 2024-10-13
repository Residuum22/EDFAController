#pragma once

/**
 * @brief This function initialize the VOA driver in forward direction.
 * The function is initialize MCPWM pheripheral in the right order.
 * More details can be found in the ESP-IDF documentation.
 * 
 */
void peltier_control_init_cooling();

/**
 * @brief This function enable the output signals in forward direction.
 * 
 */
void peltier_control_enable_cooling();

/**
 * @brief This function enable the output signals in reverse direction.
 * 
 */
void peltier_control_disable_cooling();

/**
 * @brief This function is the main VOA task. In this function fwd and rev
 * MCPWM pheriheral and moving VOA to the end positons. In the main loop if the attenuation
 * value is sent then this function is responsible to set the correct value.
 * 
 * @param pvParameters Unused - FreeRTOS parameter
 */
void peltier_control_task(void *pvParameters);
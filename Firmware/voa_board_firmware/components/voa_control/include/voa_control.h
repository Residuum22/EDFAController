#pragma once

void voa_control_init_fwd();

void voa_control_init_rev();

void voa_control_enable_fwd();

void voa_control_enable_rev();

void voa_control_disable_fwd();

void voa_control_disable_rev();

void voa_control_adc_init();

void voa_control_set_attenuation_zero();

void voa_control_set_attenuation_max();

void voa_control_task(void *pvParameters);
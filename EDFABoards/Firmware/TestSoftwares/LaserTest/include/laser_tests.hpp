#ifndef __HEADER_VOA_TESTS_H__
#define __HEADER_VOA_TESTS_H__

#include <Arduino.h>

typedef enum testID_e {
  TEST_IDLE,
  TEST_LED,
  TEST_TEMP,
  TEST_MONITOR_DIODE,
  TEST_PELTIER_FWD,
  TEST_PELTIER_REV,
  TEST_LASER1_CONTROL,
  TEST_LASER2_CONTROL,
  NUMBER_OF_TESTS
} testID;


String test_help(void);

void test_leds_init(void);
void test_leds_set_default();
void test_leds(void);

void test_temperature_sensor();
void test_monitor_diode();

void test_peltier_init();
void test_peltier_channel_idle();
void test_peltier_channel_fwd();
void test_peltier_channel_rev();

void test_laser_idle();
void test_laser1_control(uint8_t value);
void test_laser2_control();

#endif //__HEADER_VOA_TESTS_H__
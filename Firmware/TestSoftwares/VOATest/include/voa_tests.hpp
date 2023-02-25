#ifndef __HEADER_VOA_TESTS_H__
#define __HEADER_VOA_TESTS_H__

#include <Arduino.h>

typedef enum testID_e {
  TEST_IDLE,
  TEST_LED,
  TEST_ADC,
  TEST_MOTOR_FWD,
  TEST_MOTOR_REV,
  NUMBER_OF_TESTS
} testID;


String test_help(void);

void test_leds_init(void);
void test_leds_set_default();
void test_leds(void);

void test_adc();

void test_motor_init();

void test_motor_channel_idle();

void test_motor_channel_fwd();

void test_motor_channel_rev();

#endif //__HEADER_VOA_TESTS_H__
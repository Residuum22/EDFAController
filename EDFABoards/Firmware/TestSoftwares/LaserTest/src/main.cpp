#include <Arduino.h>
#include <Regexp.h>
#include "laser_tests.hpp"

static char testCase;
static testID currentTest = TEST_IDLE;
MatchState ms;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  ms.Target("[1234567|e|+|-]");

  test_leds_init();
  test_peltier_init();
  test_laser_idle();

  Serial.println(test_help());
}

static uint8_t laser_value = 0;
void loop()
{
  if (Serial.available() != 0)
  {
    testCase = Serial.read();
    if (ms.Match(&testCase) == REGEXP_MATCHED)
    {
      if (testCase == 'e')
      {
        currentTest = TEST_IDLE;
        test_peltier_channel_idle();
        test_leds_set_default();
        test_laser_idle();
        Serial.println("Test ended!");
        Serial.println();
      } 
      else if (testCase == '+')
      {
        if (laser_value < 200)
        {
          laser_value += 100;
          Serial.println("Laser value: " + String(laser_value));
        }
      } 
      else if (testCase == '-')
      {
        if (laser_value > 0)
        {
          laser_value -= 100;
          Serial.println("Laser value: " + String(laser_value));
        }
      }
      else
      {
        currentTest = (testID)(testCase - '0');
        Serial.println("Currnet test: " + String(currentTest));
      }
    }
  }

  switch (currentTest)
  {
  case TEST_LED:
    test_leds();
    break;

  case TEST_TEMP:
    test_temperature_sensor();
    break;

  case TEST_MONITOR_DIODE:
    test_monitor_diode();
    break;

  case TEST_PELTIER_FWD:
    test_peltier_channel_fwd();
    break;

  case TEST_PELTIER_REV:
    test_peltier_channel_rev();
    break;

  case TEST_LASER1_CONTROL:
    test_laser1_control(laser_value);
    break;

  case TEST_LASER2_CONTROL:
    break;

  default:
    break;
  }
}
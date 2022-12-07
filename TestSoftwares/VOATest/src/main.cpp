#include <Arduino.h>
#include <Regexp.h>
#include "voa_tests.hpp"

static char testCase;
static testID currentTest = TEST_IDLE;
MatchState ms;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  ms.Target("[1234|e]");

  test_leds_init();
  test_motor_init();

  Serial.println(test_help());
}

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
        test_motor_channel_idle();
        test_leds_set_default();
        Serial.println("Test ended!");
        Serial.println();
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

  case TEST_ADC:
    test_adc();
    break;

  case TEST_MOTOR_FWD:
    test_motor_channel_fwd();
    
    break;

  case TEST_MOTOR_REV:
    test_motor_channel_rev();
    break;

  default:
    break;
  }
}
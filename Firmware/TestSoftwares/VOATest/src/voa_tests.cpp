#include "voa_tests.hpp"
#include <Arduino.h>

static const String helpMessage = {
"USAGE:\n"
"Type the test number into the terminal to run a test.\n"
"If you want to exit the test type 'e'.\n"
"\n"
"Available options:\n"
"1 - Indicator led test\n"
"2 - ADC test\n"
"3 - Motor driver test (forward)\n"
"4 - Motor driver test (reversed)\n"
"e - Exit current test"
};

/* USER DEFINES */
#define LED_GREEN_PIN 5
#define LED_RED_PIN 18

#define ADC_PIN 4

#define MOTOR_CHANNEL_FWD 0
#define MOTOR_CHANNEL_REV 1

#define MOTOR_CHANNELA_FWD_PIN 32
#define MOTOR_CHANNELA_REV_PIN 33
    
#define MOTOR_CHANNELB_FWD_PIN 25
#define MOTOR_CHANNELB_REV_PIN 26

/* USER MACROS */
#define CONVERT_DUTY_CYCLE(x) (int)(x * UINT8_MAX / 100)

String test_help()
{
    return helpMessage;
}

void test_leds_init()
{
    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
}

static int ledState = LOW;

void test_leds()
{
    digitalWrite(LED_GREEN_PIN, ledState);
    digitalWrite(LED_RED_PIN, !ledState);
    ledState = !ledState;
    sleep(1);
}

void test_leds_set_default()
{
    ledState = LOW;
    digitalWrite(LED_GREEN_PIN, ledState);
    digitalWrite(LED_RED_PIN, ledState);
}

void test_adc()
{
    Serial.println("Value: " + String((int)analogRead(ADC_PIN)));
    sleep(1);
}

void test_motor_init()
{
    const int freq = 1000;
    const int resolution = 8;

    // configure LED PWM functionalitites
    ledcSetup(MOTOR_CHANNEL_FWD, freq, resolution);
    ledcSetup(MOTOR_CHANNEL_REV, freq, resolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(MOTOR_CHANNELA_FWD_PIN, MOTOR_CHANNEL_FWD);
    ledcAttachPin(MOTOR_CHANNELA_REV_PIN, MOTOR_CHANNEL_REV);
    ledcAttachPin(MOTOR_CHANNELB_FWD_PIN ,MOTOR_CHANNEL_FWD);
    ledcAttachPin(MOTOR_CHANNELB_REV_PIN, MOTOR_CHANNEL_REV);
}

void test_motor_channel_fwd()
{
    ledcWrite(MOTOR_CHANNEL_FWD, CONVERT_DUTY_CYCLE(50));
    sleep(1);
}

void test_motor_channel_rev()
{
    ledcWrite(MOTOR_CHANNEL_REV, CONVERT_DUTY_CYCLE(50));
    sleep(1);
}

void test_motor_channel_idle()
{
    ledcWrite(MOTOR_CHANNEL_REV, CONVERT_DUTY_CYCLE(0));
    ledcWrite(MOTOR_CHANNEL_FWD, CONVERT_DUTY_CYCLE(0));
}

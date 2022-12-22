#include "laser_tests.hpp"
#include <Arduino.h>

static const String helpMessage = {
"USAGE:\n"
"Type the number into the terminal to run a test. If you want to exit from the test type 'e'.\n"
"\n"
"Available options:\n"
"1 - Indicator LED test\n"
"2 - Temperature sensor test (ADC)\n"
"3 - Monitor diode test (ADC)\n"
"4 - Peltiers test (cooling)\n"
"5 - Peltier test (heating)\n"
"6 - Laser1 control test (DAC)\n"
"7 - Laser2 control test (DAC)\n"
};

/* USER DEFINES */
#define LED_GREEN_PIN 5
#define LED_RED_PIN 18

#define TEMP1_PIN 2
#define TEMP2_PIN 4

#define MONITOR_DIODE1_PIN 34
#define MONITOR_DIODE2_PIN 35

#define PELTIER_CHANNEL_FWD 0
#define PELTIER_CHANNEL_REV 1

#define PELTIER1_FWD_PIN 21
#define PELTIER1_REV_PIN 19
    
#define PELTIER2_FWD_PIN 23
#define PELTIER2_REV_PIN 22

#define LASER1_CONTROL_PIN  26
#define LASER2_CONTROL_PIN  25

/* USER MACROS */
#define CONVERT_DUTY_CYCLE(x) (int)(x * UINT8_MAX / 100)
#define VOLTAGE_DIVIDER_LASER1 680/(10000+680)
#define CONVERT_MA_TO_VALUE(x) (int)(0.0647 * 3.3 * x * 1000 / 255)

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

void test_temperature_sensor()
{
    analogSetAttenuation(ADC_11db);
    Serial.println("Temp1 value: " + String((int)analogRead(TEMP1_PIN) * 3.3 / 4096));
    Serial.println("Temp2 value: " + String((int)analogRead(TEMP2_PIN) * 3.3 / 4096));
    sleep(1);
}

void test_monitor_diode()
{
    Serial.println("Monitor1 value: " + String((int)analogRead(MONITOR_DIODE1_PIN)));
    Serial.println("Monitor2 value: " + String((int)analogRead(MONITOR_DIODE2_PIN)));
    sleep(1);
}

void test_peltier_init()
{
    const int freq = 1000;
    const int resolution = 8;

    // configure LED PWM functionalitites
    ledcSetup(PELTIER_CHANNEL_FWD, freq, resolution);
    ledcSetup(PELTIER_CHANNEL_REV, freq, resolution);

    // attach the channel to the GPIO to be controlled
    ledcAttachPin(PELTIER1_FWD_PIN, PELTIER_CHANNEL_FWD);
    ledcAttachPin(PELTIER1_REV_PIN, PELTIER_CHANNEL_REV);
    ledcAttachPin(PELTIER2_FWD_PIN ,PELTIER_CHANNEL_FWD);
    ledcAttachPin(PELTIER2_REV_PIN, PELTIER_CHANNEL_REV);
}

void test_peltier_channel_fwd()
{
    ledcWrite(PELTIER_CHANNEL_FWD, CONVERT_DUTY_CYCLE(50));
    sleep(1);
}

void test_peltier_channel_rev()
{
    ledcWrite(PELTIER_CHANNEL_REV, CONVERT_DUTY_CYCLE(50));
    sleep(1);
}

void test_peltier_channel_idle()
{
    ledcWrite(PELTIER_CHANNEL_REV, CONVERT_DUTY_CYCLE(0));
    ledcWrite(PELTIER_CHANNEL_FWD, CONVERT_DUTY_CYCLE(0));
}

void test_laser1_control(uint8_t value)
{
    dacWrite(LASER1_CONTROL_PIN, value);
}

void test_laser2_control()
{

}

void test_laser_idle()
{
    dacWrite(LASER1_CONTROL_PIN, 0);
    dacWrite(LASER2_CONTROL_PIN, 0);
}
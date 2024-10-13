#include "led_indicator.h"
#include "led_indicator_blink_default.h"

/*********************************** Config Blink List in Different Conditions ***********************************/
/**
 * @brief State off 
 *
 */
static const blink_step_t indicate_none[] = {
    {LED_BLINK_HOLD, LED_STATE_OFF, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Indicate error
 *
 */
static const blink_step_t indicate_error[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Indicate WIFI connecting
 *
 */
static const blink_step_t wifi_connecting[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 200},
    {LED_BLINK_HOLD, LED_STATE_OFF, 800},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Indicate MQTT connecting
 *
 */
static const blink_step_t mqtt_connecting[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Indicate MQTT connected
 *
 */
static const blink_step_t mqtt_connected[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief LED indicator blink lists, the index like BLINK_FACTORY_RESET defined the priority of the blink
 *
 */
blink_step_t const *default_led_indicator_blink_lists[] = {
    [BLINK_INDICATE_NONE] = indicate_none,
    [BLINK_INDICATE_ERROR] = indicate_error,
    [BLINK_WIFI_CONNECTING] = wifi_connecting,
    [BLINK_MQTT_CONNECTING] = mqtt_connecting,
    [BLINK_MQTT_CONNECTED] = mqtt_connected,
    [BLINK_MAX] = NULL,
};

/* LED blink_steps handling machine implementation */
const int DEFAULT_BLINK_LIST_NUM = (sizeof(default_led_indicator_blink_lists) / sizeof(default_led_indicator_blink_lists[0]));
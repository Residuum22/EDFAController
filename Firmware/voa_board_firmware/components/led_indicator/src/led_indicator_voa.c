#include "led_custom.h"

#include "common_gpio.h"
#include "led_indicator.h"
#include "led_indicator_blink_default.h"


static led_indicator_handle_t led_state_handle;
static led_indicator_handle_t led_error_handle;

static int state_led_blink_prev = BLINK_MAX;
static int error_led_blink_prev = BLINK_MAX;


void voa_indicator_init()
{
    led_indicator_gpio_config_t led_state_gpio_config = {
        .is_active_level_high = true,
        .gpio_num = INDICATOR_GREEN_GPIO,
    };
    led_indicator_config_t led_state_config = {
        .mode = LED_GPIO_MODE,
        .led_indicator_gpio_config = &led_state_gpio_config,
        .blink_lists = default_led_indicator_blink_lists,
        .blink_list_num = DEFAULT_BLINK_LIST_NUM,
    };
    led_state_handle = led_indicator_create(&led_state_config);

    led_indicator_gpio_config_t led_error_gpio_config = {
        .is_active_level_high = true,
        .gpio_num = INDICATOR_RED_GPIO,
    };
    led_indicator_config_t led_error_config = {
        .mode = LED_GPIO_MODE,
        .led_indicator_gpio_config = &led_error_gpio_config,
        .blink_lists = default_led_indicator_blink_lists,
        .blink_list_num = DEFAULT_BLINK_LIST_NUM,
    };
    led_error_handle = led_indicator_create(&led_error_config);
}

void voa_indicator_set_state(int blink_type)
{
    if (state_led_blink_prev != BLINK_MAX)
    {
        led_indicator_stop(led_state_handle, state_led_blink_prev);
    }
    led_indicator_start(led_state_handle, blink_type);
    state_led_blink_prev = blink_type;
}

void voa_indicator_set_error(int blink_type)
{
    voa_indicator_set_state(BLINK_PROVISIONED);
    if (error_led_blink_prev != BLINK_MAX)
    {
        led_indicator_stop(led_error_handle, error_led_blink_prev);
    }
    led_indicator_start(led_error_handle, blink_type);
    error_led_blink_prev = blink_type;
}
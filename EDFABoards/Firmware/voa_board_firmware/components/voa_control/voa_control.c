#include "common_gpio.h"
#include "voa_control.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/gpio.h"

#include "led_indicator.h"
#include "led_indicator_blink_default.h"
#include "led_indicator_voa.h"

#include "mqtt3.h"

#include <math.h>

static const char *TAG = "VOA_CONTROL";

//*****************************************************************************
// Defines START
//*****************************************************************************
// These defines are created for MCPWM pheripheral
#define TIMER_RESOLUTION_HZ 1000000             // 1MHz, 1us per tick
#define TIMER_PERIOD        50000               // 50.000 ticks => T=50ms => F=20 Hz
#define COMPARE_VALUE       (TIMER_PERIOD / 4)  // Because square signal and up, down counter timer

// https://www.calculator.net/slope-calculator.html
#define SLOPE                  100
#define CONVERT_DB_TO_MV(x)    (x * SLOPE + voa_min_voltage)

// This should be a low value
// If it is low, the noise can trigger the end values
// If it is to high, the motor cant be set into the correct position
#if CONFIG_USE_EPS_VAR
int adc_position_eps;
#else
#define ADC_POSITION_EPS 20
#endif

#define TIMER_CHECK_INTERVALL 1000
//*****************************************************************************
// Defines END
//*****************************************************************************

//*****************************************************************************
// Private variables and structs START
//*****************************************************************************

// This variable is located in the main component (main.c file)
// Used for IPC between MQTT and this VOA controller component.
extern QueueHandle_t voa_attenuation_queue;

// ESP32 only have 2 MCPWM groups, each have 2 generator
typedef enum voa_mcpwm_group_id
{
    VOA_MCPWM_GROUP_0 = 0,
    VOA_MCPWM_GROUP_1,
    VOA_MCPWM_GROUP_MAX
} voa_mcpwm_group_id_t;

// MCPWM generator handlers
// Concept: One group for the forwards and backwards, which need to be
//          deinitialized. First 2 generators are for the coilA the 
//          last 2 are for the coilB.
static mcpwm_gen_handle_t generators[4];

// Comparator handle -> Need to be changed in runtime.
static mcpwm_cmpr_handle_t comparators[2];


// ADC1 oneshot configuration
static adc_cali_handle_t adc1_cali_handle = NULL;
static adc_oneshot_unit_handle_t adc1_handle;


// Automatic end position mapper software timer handler
static TimerHandle_t sw_timer;

// Automatic end position mapper variables
static int voa_max_voltage;
static int voa_min_voltage;
static bool voa_stopped = false;

// General use for the ADC
static int current_adc_voltage = 0;
static int last_adc_voltage = 0;

//*****************************************************************************
// Private variables and structs END
//*****************************************************************************

//*****************************************************************************
// Static functions START
//*****************************************************************************

//-----------------------------------------------------------------------------
// MCPWM part
//-----------------------------------------------------------------------------

/**
 * @brief This function configures stepper motor coilA PWM signal.
 * The current in the coils need to be inverted every new cycle.
 * Don't use this function directly.
 * 
 * @param gens Generator for the coilA. (should gen0 and gen1) use 
 * generators variable here because of the array indexing.
 *  
 */
static void _coilA_generator_action_config(mcpwm_gen_handle_t *gens)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        gens[0],
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        gens[1],
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()));
}

/**
 * @brief This function creates the coilB signals. In forward mode the the current need to
 * be delayed with 90 degree. Check /docs/mcpwm.md for the documentation.
 * Don't use this function directly.
 * 
 * @param gens Generator for the coilB (use generators[4] because of the indexing)
 * @param cmps Comparator for the coilB
 */
static void _coilB_fwd_generator_action_config(mcpwm_gen_handle_t *gens, mcpwm_cmpr_handle_t *cmps)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        gens[2],
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmps[0], MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmps[0], MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        gens[3],
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmps[1], MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmps[1], MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}

/**
 * @brief This function creates the coilB signals. In forward mode the the current need to
 * be forward with 90 degree. Check /docs/mcpwm.md for the documentation.
 * Don't use this function directly.
 * 
 * @param gens Generator for the coilB (use generators[4] because of the indexing)
 * @param cmps Comparator for the coilB
 */
static void _coilB_rev_generator_action_config(mcpwm_gen_handle_t *gens, mcpwm_cmpr_handle_t *cmps)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        gens[2],
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmps[0], MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmps[0], MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        gens[3],
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmps[1], MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmps[1], MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}

/**
 * @brief This function will set the VOA motor into forward mode.
 *
 */
static void set_voa_forward_mode()
{
    _coilA_generator_action_config(generators);
    _coilB_fwd_generator_action_config(generators, comparators);
}

/**
 * @brief This function will set the VOA motor into reverse moce.
 * 
 */
static void set_voa_reverse_mode()
{
    _coilA_generator_action_config(generators);
    _coilB_rev_generator_action_config(generators, comparators);
}

/**
 * @brief This function is initializing the MCPWM pheripheral timer part. According to the ESP32
 * documentation, every ESP32 has two MCPWM pheripheral which is indexed by GROUP_ID (ID0 and ID1).
 * Before you start dig deeper into this "beautiful :)" code see the ESP-IDF MCPWM documentation and
 * read it carefully.
 *
 * @param mcpwm_group_id Groupd ID because there is two MCPWM pheripheral.
 * @param timer MCPWM timer handler
 */
static void _timer_init(uint8_t mcpwm_group_id, mcpwm_timer_handle_t *timer)
{
    ESP_LOGI(TAG, "Create timer for group %d", mcpwm_group_id);
    mcpwm_timer_config_t timer_config = {
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .group_id = mcpwm_group_id,
        .resolution_hz = TIMER_RESOLUTION_HZ,
        .period_ticks = TIMER_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP_DOWN,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, timer));
}

/**
 * @brief This function create a new operator in the given MCPWM group.
 *
 * @param mcpwm_group_id Group ID
 * @param operators MCPWM operator handler
 */
static void _operator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators)
{
    ESP_LOGI(TAG, "Create operators for group %d", mcpwm_group_id);
    mcpwm_operator_config_t operator_config = {
        .group_id = mcpwm_group_id, // operator should be in the same group of the above timers
    };
    for (int i = 0; i < 2; ++i)
    {
        // Create two operator (later for two generator)
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operators[i]));
    }
}

/**
 * @brief This function connect the given timer with the operator in the MCPWM group.
 *
 * @param mcpwm_group_id Group ID
 * @param timer MCPWM timer handler
 * @param operators MCPWM operator handler
 */
static void _connect_timer_operator(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer, mcpwm_oper_handle_t *operators)
{
    ESP_LOGI(TAG, "Connect timers and operators with each other for group %d", mcpwm_group_id);
    for (int i = 0; i < 2; i++)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operators[i], timer));
    }
}

/**
 * @brief This function initialize the comparator in the given MCPWM group's operator.
 * The compare value is set FREQ/4 because in the software the timer is configured as an
 * UP and DOWN counter. The timer compare config is tez and tep, so if the timer over/under flow
 * everything is starting again. The compare value must be set between the increment/decrement slope.
 * Because we need a symmetric square wave (50% duty), the compare value must be set in the FREQ/4 positions.
 * See /docs/mcpwm.md
 *
 * @param mcpwm_group_id Group ID - Only for ESP_LOG
 * @param operators MCPWM operator handler
 * @param comparator MCPWM comprator handler
 */
static void _comparator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators, mcpwm_cmpr_handle_t *comparators)
{
    ESP_LOGI(TAG, "Create comparator for second operator for group %d", mcpwm_group_id);
    // Configuration to use comparator with the overflow and underflow signals. Software sync disable.
    mcpwm_comparator_config_t compare_config = {
        .flags.update_cmp_on_tez = true,
        .flags.update_cmp_on_tep = true,
        .flags.update_cmp_on_sync = false,
    };

    // Create comparator in the operator
    for (uint8_t i = 0; i < 2; i++)
    {
        ESP_ERROR_CHECK(mcpwm_new_comparator(operators[i], &compare_config, &comparators[i]));
        ESP_LOGI(TAG, "Set comparator value: %d", COMPARE_VALUE);
        // Set the comparator value. (More details in the brief)
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparators[i], COMPARE_VALUE));
    }
}

/**
 * @brief Init MCPWM generator. This function is the last element of the MCPWM peripheral, which is doing the
 * real signal generator on the output pins.
 *
 * @param mcpwm_group_id Group ID - Only for ESP_LOG
 * @param operators Operator handler
 * @param generators Generator handler
 * @param gen_gpio GPIO output instance, where the signal will be represented
 * @param comparator Comparator handler
 */
static void _generator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators, mcpwm_gen_handle_t *generators, const uint8_t *gen_gpio)
{
    ESP_LOGI(TAG, "Create generators for group %d", mcpwm_group_id);
    mcpwm_generator_config_t gen_config = {};
    for (int i = 0; i < 4; i++)
    {
        // Quick info:
        // 2 operator pointer, four pin number, four generators 
        // 2 generator in the first operator, two generator in the second.
        gen_config.gen_gpio_num = gen_gpio[i];
        ESP_ERROR_CHECK(mcpwm_new_generator(operators[i % 2], &gen_config, &generators[i]));
    }
}

/**
 * @brief Start MCPWM timer.
 *
 * @param mcpwm_group_id Group ID
 * @param timer Timer handler
 */
static void _start_timer(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer)
{
    ESP_LOGI(TAG, "Start timer for group %d", mcpwm_group_id);
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

//-----------------------------------------------------------------------------
// End position mapping and emergency stop part
//-----------------------------------------------------------------------------

/**
 * @brief This function initialize the SENSOR_VP IO36 pin as an input
 *
 */
static void inline voa_adc_gpio_init()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_36),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
}

/**
 * @brief This function compares two values which is given with the parameter list.
 * If the value is between in a range of EPSILON (this is a small number), measured in
 * value of the ADC this number is between 5-30.
 *
 * @param current_voltage Current ADC value
 * @param last_voltage Last stored ADC value
 * @return true If the voa pot is chaging without the eps value.
 * @return false If the voa pot is NOT changing within the eps value.
 */
static bool voa_control_is_moving(int current_voltage, int last_voltage)
{
    ESP_LOGI(TAG, "Checking VOA position values");
    int difference = abs(last_voltage - current_voltage);

#if CONFIG_USE_EPS_VAR
    if (difference <= adc_position_eps)
#else
    if (difference <= ADC_POSITION_EPS)
#endif
    {
        ESP_LOGI(TAG, "[WARNING] VOA value is not changing... Current ADC measurement: %d", current_voltage);
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "VOA value is changing... Current ADC measurement: %d", current_voltage);
        return true;
    }
}

/**
 * @brief This function is saving the last ADC value and checking if the
 * voa is stopped or reached the end position. This function can be called as
 * a software timer callback or just saving the current VOA position in the RAM.
 * 
 * @note This function only can be called in the timer.
 * @return true
 * @return false
 */
static void voa_control_check_if_stopped(TimerHandle_t xTimer)
{
    int adc_tmp;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_tmp));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_tmp, &current_adc_voltage));

    if (!voa_control_is_moving(current_adc_voltage, last_adc_voltage))
    {
        // Emergency shutdown
        voa_control_disable_output();
        voa_stopped = true;
    }
    last_adc_voltage = current_adc_voltage;
}
//*****************************************************************************
// Static functions END
//*****************************************************************************

void voa_control_init(void)
{
    mcpwm_timer_handle_t timer;
    _timer_init(VOA_MCPWM_GROUP_0, &timer);

    mcpwm_oper_handle_t operators[2];
    _operator_init(VOA_MCPWM_GROUP_0, operators);

    _connect_timer_operator(VOA_MCPWM_GROUP_0, timer, operators);

    _comparator_init(VOA_MCPWM_GROUP_0, operators, comparators);

    const uint8_t gen_gpios[4] = {VOA_FWD_A_PIN, VOA_REV_A_PIN, VOA_FWD_B_PIN, VOA_REV_B_PIN};
    _generator_init(VOA_MCPWM_GROUP_0, operators, generators, gen_gpios);

    voa_control_disable_output();

    _start_timer(VOA_MCPWM_GROUP_0, timer);
}

void voa_control_enable_output()
{
    ESP_LOGI(TAG, "Enable output for forward VOA");
    for (int i = 0; i < 4; ++i)
    {
        // remove the force level on the generator, so that we can see the PWM again
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(generators[i], -1, true));
    }
}

void voa_control_disable_output()
{
    ESP_LOGI(TAG, "Disable output for forward VOA");
    for (int i = 0; i < 4; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(generators[i], 0, true));
    }
}

void voa_control_adc_init()
{
    //-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC2 Calibration Init---------------//
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_13,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc1_cali_handle));

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_13,
        .atten = ADC_ATTEN_DB_11, // Need to measure maximum 2.4V 
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
}

void voa_control_set_attenuation_zero()
{
    // Temporary value to read voa before converting it into mV
    int voa_adc_value;

    // Start to decrease attenuation
    voa_stopped = false;

    ESP_LOGI(TAG, "Set generator in reverse mode!");
    set_voa_reverse_mode();

    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "Enable VOA output...");
    voa_control_enable_output();

    // Start timer
    // Before timer start current_adc_value need to be cleared
    current_adc_voltage = 0;
    xTimerStart(sw_timer, 0);

    // Global variable for communication the timer will set this true
    while (!voa_stopped)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    voa_stopped = false;
    // After this the voa should be stopped.

    xTimerStop(sw_timer, 0);

    // Store min value

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &voa_adc_value));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, voa_adc_value, &voa_min_voltage));
    ESP_LOGI(TAG, "[VOA_MIN_VOLTAGE]: %d", voa_min_voltage);
}

void voa_control_set_attenuation_max()
{
    // Temporary value to read voa before converting it into mV
    int voa_adc_value;

    // Start to decrease attenuation
    voa_stopped = false;

    ESP_LOGI(TAG, "Set generator in forward mode!");
    set_voa_forward_mode();

    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "Enable VOA output...");
    voa_control_enable_output();

    // Start timer
    // Before timer start current_adc_value need to be cleared
    current_adc_voltage = 0;
    xTimerStart(sw_timer, 0);

    // This value is modified with the timer handler function namely voa_control_check_if_stopped
    while (!voa_stopped)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    voa_stopped = false;
    // After this the voa should be stopped.

    xTimerStop(sw_timer, 0);

    // Store min value
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &voa_adc_value));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, voa_adc_value, &voa_max_voltage));
    ESP_LOGI(TAG, "[VOA_MIN_VOLTAGE]: %d", voa_max_voltage);
}

void voa_control_task(void *pvParameters)
{
    uint8_t attenuation = 0;
    int adc_value = 0;
    int adc_voltage = 0;

    voa_control_init();

    ESP_LOGI(TAG, "Initialize ADC...");
    voa_adc_gpio_init();
    voa_control_adc_init();

    // Init timer for error/end position ending
    sw_timer = xTimerCreate("VOA Timer", pdMS_TO_TICKS(TIMER_CHECK_INTERVALL), pdTRUE, (void *)1, &voa_control_check_if_stopped);

    // Map min and max value of the voa.
    voa_control_set_attenuation_max();
    voa_control_set_attenuation_zero();

    for (;;)
    {   
        xQueueReceive(voa_attenuation_queue, &attenuation, portMAX_DELAY);
        int expected_mv = CONVERT_DB_TO_MV(attenuation);
        ESP_LOGI(TAG, "Attenuation in mV: %d", expected_mv);

        // Check if the attenuation is greater than the maximum value
        if (expected_mv > voa_max_voltage)
        {
            expected_mv = voa_max_voltage;
        }
        // Check if the attenuation is less than the minimum value
        else if (expected_mv < voa_max_voltage)
        {
            expected_mv = voa_max_voltage;
        }

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_value, &adc_voltage));
        ESP_LOGI(TAG, "Current ADC voltage: %d mV | Expected voltage: %d mV", adc_voltage, expected_mv);
        // Before timer start current_adc_value need to be cleared
        current_adc_voltage = 0;
        voa_stopped = false;
        xTimerStart(sw_timer, 0);

        if (adc_voltage < expected_mv)
        {
            ESP_LOGI(TAG, "Move forward...");
            // if the initial state of the VOA pot is less then the expected -> move forward
            set_voa_forward_mode();
            voa_control_enable_output();

            while (adc_voltage < expected_mv && !voa_stopped)
            {
                vTaskDelay(10);
                // Wait here until the last value is updated
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
                ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_value, &adc_voltage));
            }
        }
        else
        {
            ESP_LOGI(TAG, "Move backwards...");
            // Move backward
            set_voa_reverse_mode();
            voa_control_enable_output();

            while (adc_voltage > expected_mv && !voa_stopped)
            {
                vTaskDelay(10);
                // Wait here until the last value is updated
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
                ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_value, &adc_voltage));
            }
        }

        xTimerStop(sw_timer, 0);
        voa_control_disable_output();
        ESP_LOGI(TAG, "Current ADC voltage: %d mV | Expected voltage: %d mV", adc_voltage, expected_mv);
    }
}

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

static const char *TAG = "VOA_CONTROL";

//*****************************************************************************
// Defines START
//*****************************************************************************
// These defines are created for MCPWM pheripheral
#define TIMER_RESOLUTION_HZ 1000000    // 1MHz, 1us per tick
#define TIMER_PERIOD 1000              // 1000 ticks, 1ms
#define COMPARE_VALUE TIMER_PERIOD / 4 // Because square signal and up, down counter timer

// These defines are created for the ADC value and attenuation conversion
#define POT_VOLTAGE 4095
#define MAX_VOA_ATTENUATION_IN_V POT_VOLTAGE * 0.98
#define MIN_VOA_ATTENUATION_IN_V POT_VOLTAGE * 0.4
// https://www.calculator.net/slope-calculator.html
#define SLOPE 122.85
#define CONVERT_DB_TO_VOLTAGE(x) (x * SLOPE + MIN_VOA_ATTENUATION_IN_V)
#define CONVERT_DB_TO_VALUE(x) (x * SLOPE + voa_min_adc_value)

// This should be a low value
// TODO: need to find the good value
// If it is low, the noise can trigger the end values
// If it is to high, the motor cant be set into the correct position
#define ADC_POSITION_EPS 20

// TODO: This period between 1-3 to prevent the voa coil burn down.
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
// Concept: One group for forward and one group for reverse
static mcpwm_gen_handle_t fwd_generators[2];
static mcpwm_gen_handle_t rev_generators[2];

// ADC1 oneshot configuration
static adc_cali_handle_t adc1_cali_handle = NULL;
static bool do_calibration1;
static adc_oneshot_unit_handle_t adc1_handle;

// Automatic end position mapper variables
static int voa_max_adc_value = 4095;
static int voa_min_adc_value = 0;
static bool voa_stopped = false;

// Automatic end position mapper software timer handler
static TimerHandle_t sw_timer;

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
 * @brief This fuction is setting up the MCPWM periheral comparators. According to this code
 * the MCPWM pheripheral can make two square signal, which phases are shifted with 90 degree.
 *
 * @param gena Generator A (first generator handler of the configurated MCPWM)
 * @param genb Generator B (second generator handler of the configurated MCPWM)
 * @param comp Comparator (comparator handler for the configured MCPWM)
 */
static void gen_action_config(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t comp)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        gena,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        genb,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comp, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, comp, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}

/**
 * @brief This function is initializibg the MCPWM pheripheral timer part. According to the ESP32
 * documentation, every ESP32 has two MCPWM pheripheral which is indexed by GROUP_ID (ID0 and ID1).
 * Before you start dig deeper into this "beautiful :)" code see the ESP-IDF MCPWM documentation and 
 * read it carefully.
 *
 * @param mcpwm_group_id Groupd ID because there is two MCPWM pheripheral.
 * @param timer MCPWM timer handler
 */
static void timer_init(uint8_t mcpwm_group_id, mcpwm_timer_handle_t *timer)
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
static void operator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators)
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
static void connect_timer_operator(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer, mcpwm_oper_handle_t *operators)
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
 * 
 * @param mcpwm_group_id Group ID - Only for ESP_LOG
 * @param operators MCPWM operator handler
 * @param comparator MCPWM comprator handler
 */
static void comparator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators, mcpwm_cmpr_handle_t *comparator)
{
    ESP_LOGI(TAG, "Create comparator for second operator for group %d", mcpwm_group_id);
    // Configuration to use comparator with the overflow and underflow signals. Software sync disable.
    mcpwm_comparator_config_t compare_config = {
        .flags.update_cmp_on_tez = true,
        .flags.update_cmp_on_tep = true,
        .flags.update_cmp_on_sync = false,
    };

    // Create comparator in the operator
    ESP_ERROR_CHECK(mcpwm_new_comparator(operators[1], &compare_config, comparator));
    ESP_LOGI(TAG, "Set comparator value: %d", COMPARE_VALUE);
    // Set the comparator value. (More details in the brief)
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(*comparator, COMPARE_VALUE));
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
static void generator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators, mcpwm_gen_handle_t *generators, const uint8_t *gen_gpio, mcpwm_cmpr_handle_t comparator)
{
    ESP_LOGI(TAG, "Create generators for group %d", mcpwm_group_id);
    mcpwm_generator_config_t gen_config = {};
    for (int i = 0; i < 2; i++)
    {
        gen_config.gen_gpio_num = gen_gpio[i];
        ESP_ERROR_CHECK(mcpwm_new_generator(operators[i], &gen_config, &generators[i]));
    }
}

/**
 * @brief Start MCPWM timer.
 * 
 * @param mcpwm_group_id Group ID
 * @param timer Timer handler 
 */
static void start_timer(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer)
{
    ESP_LOGI(TAG, "Start timer for group %d", mcpwm_group_id);
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

//-----------------------------------------------------------------------------
// End position mapping and emergency stop part
//-----------------------------------------------------------------------------

/**
 * @brief This function compares two values which is given with the parameter list.
 * If the value is between in a range of EPSILON (this is a small number), measured in 
 * value of the ADC this number is between 5-30.
 * 
 * @param current_value Current ADC value
 * @param last_value Last stored ADC value
 * @return true If the voa pot is chaging without the eps value.
 * @return false If the voa pot is NOT changing within the eps value.
 */
static bool voa_control_check_eps(int current_value, int last_value)
{
    ESP_LOGI(TAG, "Checking VOA position values");
    if ((current_value - last_value >= ADC_POSITION_EPS) || (last_value - current_value >= ADC_POSITION_EPS))
    {
        ESP_LOGI(TAG, "VOA value is changing...");
        return true;
    }
    ESP_LOGI(TAG, "[WARNING] VOA value is not changing...");
    return false;
}

/**
 * @brief This function is saving the last ADC value and checking if the 
 * voa is stopped or reached the end position. This function can be called as 
 * a software timer callback or just saving the current VOA position in the RAM.
 */
static void voa_control_check_if_stopped()
{
    static int last_adc_value = 0;
    static int current_adc_value = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &current_adc_value));

    if (!voa_control_check_eps(current_adc_value, last_adc_value))
    {
        // Emergency shutdown
        voa_control_disable_fwd();
        voa_control_disable_rev();
    }
    last_adc_value = current_adc_value;
}
//*****************************************************************************
// Static functions END
//*****************************************************************************

void voa_control_init_fwd()
{
    mcpwm_timer_handle_t timer;
    timer_init(VOA_MCPWM_GROUP_0, &timer);

    mcpwm_oper_handle_t operators[2];
    operator_init(VOA_MCPWM_GROUP_0, operators);

    connect_timer_operator(VOA_MCPWM_GROUP_0, timer, operators);

    mcpwm_cmpr_handle_t comparator;
    comparator_init(VOA_MCPWM_GROUP_0, operators, &comparator);

    const uint8_t gen_gpios[2] = {VOA_FWD_A_PIN, VOA_FWD_B_PIN};
    generator_init(VOA_MCPWM_GROUP_0, operators, fwd_generators, gen_gpios, comparator);

    voa_control_disable_fwd();

    ESP_LOGI(TAG, "Set generator actions on timer and compare event");
    gen_action_config(fwd_generators[0], fwd_generators[1], comparator);

    start_timer(VOA_MCPWM_GROUP_0, timer);
}

void voa_control_init_rev()
{
    mcpwm_timer_handle_t timer;
    timer_init(VOA_MCPWM_GROUP_1, &timer);

    mcpwm_oper_handle_t operators[2];
    operator_init(VOA_MCPWM_GROUP_1, operators);

    connect_timer_operator(VOA_MCPWM_GROUP_1, timer, operators);

    mcpwm_cmpr_handle_t comparator;
    comparator_init(VOA_MCPWM_GROUP_1, operators, &comparator);

    const uint8_t gen_gpios[2] = {VOA_REV_A_PIN, VOA_REV_B_PIN};
    generator_init(VOA_MCPWM_GROUP_1, operators, rev_generators, gen_gpios, comparator);

    voa_control_disable_rev();

    ESP_LOGI(TAG, "Set generator actions on timer and compare event");
    gen_action_config(rev_generators[0], rev_generators[1], comparator);

    start_timer(VOA_MCPWM_GROUP_1, timer);
}

void voa_control_enable_fwd()
{
    ESP_LOGI(TAG, "Enable output for forward VOA");
    for (int i = 0; i < 2; ++i)
    {
        // remove the force level on the generator, so that we can see the PWM again
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(fwd_generators[i], -1, true));
    }
}

void voa_control_enable_rev()
{
    ESP_LOGI(TAG, "Enable output for reverse VOA");
    for (int i = 0; i < 2; ++i)
    {
        // remove the force level on the generator, so that we can see the PWM again
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(rev_generators[i], -1, true));
    }
}

void voa_control_disable_fwd()
{
    ESP_LOGI(TAG, "Disable output for forward VOA");
    for (int i = 0; i < 2; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(fwd_generators[i], 0, true));
    }
}

void voa_control_disable_rev()
{
    ESP_LOGI(TAG, "Disable output for reverse VOA");
    for (int i = 0; i < 2; i++)
    {
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(rev_generators[i], 0, true));
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
    do_calibration1 = false;

    //-------------ADC2 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_0, &config));
}

void voa_control_set_attenuation_zero()
{
    // Call timer if stopped function on the first run
    voa_control_check_if_stopped();

    // Start to decrease attenuation
    voa_stopped = false;
    voa_control_disable_fwd();
    voa_control_enable_rev();

    // Start timer
    xTimerStart(sw_timer, 0);

    while (!voa_stopped)
        ;
    voa_stopped = false;
    // After this the voa should be stopped.

    xTimerStop(sw_timer, 0);

    // Store min value
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &voa_min_adc_value));
}

void voa_control_set_attenuation_max()
{
    // Call timer if stopped function on the first run
    voa_control_check_if_stopped();

    // Start to decrease attenuation
    voa_stopped = false;
    voa_control_enable_fwd();
    voa_control_disable_rev();

    // Start timer
    xTimerStart(sw_timer, 0);

    // This value is modified with the timer handler function namely voa_control_check_if_stopped
    while (!voa_stopped)
        ;
    voa_stopped = false;
    // After this the voa should be stopped.

    xTimerStop(sw_timer, 0);

    // Store min value
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &voa_max_adc_value));
}

void voa_control_task(void *pvParameters)
{
    uint8_t attenuation = 0;

    voa_control_init_fwd();
    voa_control_init_rev();

    int adc_value = 0;

    // TODO: Remove this
    // Configure GPIO4 as input becasse it is connected to the VOA potentiometer by default but it is unable to use
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_NUM_4),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&io_conf);
    // End of TODO

    // Init timer for error/end position ending
    sw_timer = xTimerCreate("VOA Timer", pdMS_TO_TICKS(TIMER_CHECK_INTERVALL), pdTRUE, (void *)1, &voa_control_check_if_stopped);

    voa_control_set_attenuation_zero();
    voa_control_set_attenuation_max();

    for (;;)
    {
        xQueueReceive(voa_attenuation_queue, &attenuation, portMAX_DELAY);
        ESP_LOGI(TAG, "Attenuation: %d", attenuation);
        int espected_raw = CONVERT_DB_TO_VALUE(attenuation);

        // Check if the attenuation is greater than the maximum value
        if (espected_raw > voa_max_adc_value)
        {
            espected_raw = voa_max_adc_value;
        }

        // Check if the attenuation is less than the minimum value
        if (espected_raw < voa_min_adc_value)
        {
            espected_raw = voa_min_adc_value;
        }

        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
        xTimerStart(sw_timer, 0);

        if (adc_value < espected_raw)
        {
            // if the initial state of the VOA pot is less then the expected -> move forward
            voa_control_disable_rev();
            voa_control_enable_fwd();
            while (adc_value < espected_raw || !voa_stopped)
            {
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
            }
        }
        else
        {
            // Move backward
            voa_control_disable_fwd();
            voa_control_enable_rev();
            while (adc_value > espected_raw || !voa_stopped)
            {
                ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_0, &adc_value));
            }
        }

        if (voa_stopped)
        {
            assert(false);
        }

        xTimerStop(sw_timer, 0);
        voa_control_disable_fwd();
        voa_control_disable_rev();
    }
}
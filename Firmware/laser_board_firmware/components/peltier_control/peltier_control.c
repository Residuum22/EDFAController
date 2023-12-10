#include "common_gpio.h"
#include "peltier_control.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "laser_module_adc.h"
#include "driver/gpio.h"

#include "pid_controller.h"

static const char *TAG = "PELTIER_CONTROL";

#define USE_PID_CONTROLLER  1
#define MORE_PELT_LOG       0

//*****************************************************************************
// Defines START
//*****************************************************************************
// These defines are created for MCPWM pheripheral
#define TIMER_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define TIMER_PERIOD 1000           // 1000 ticks, 1ms

#define COMPARE_VALUE TIMER_PERIOD / 4 // Because square signal and up, down counter timer

#define COMPARE_VALUE_MIN 0 // 0 %
#define COMPARE_VALUE_MAX TIMER_PERIOD / 2 // 100%
//*****************************************************************************
// Defines END
//*****************************************************************************

//*****************************************************************************
// Private variables and structs START
//*****************************************************************************

// This variable is located in the main component (main.c file)
extern QueueHandle_t peltier1_desired_temp_queue, peltier2_desired_temp_queue;

// ESP32 only have 2 MCPWM groups, each have 2 generator
typedef enum peltier_mcpwm_group_id
{
    PELTIER_MCPWM_GROUP_0 = 0,
    PELTIER_MCPWM_GROUP_1,
    PELTIER_MCPWM_GROUP_MAX
} peltier_mcpwm_group_id_t;

// MCPWM generator handlers
// Concept: One group for forward and one group for reverse
static mcpwm_gen_handle_t peltier1_cooling_generator;
static mcpwm_gen_handle_t peltier2_cooling_generator;

static mcpwm_cmpr_handle_t comparator1;
static mcpwm_cmpr_handle_t comparator2;

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
static void gen_action_config(mcpwm_gen_handle_t gena, mcpwm_cmpr_handle_t comp)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(
        gena,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(
        gena,
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
static void operator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t* operator)
{
    ESP_LOGI(TAG, "Create operators for group %d", mcpwm_group_id);
    mcpwm_operator_config_t operator_config = {
        .group_id = mcpwm_group_id, // operator should be in the same group of the above timers
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, operator));
}

/**
 * @brief This function connect the given timer with the operator in the MCPWM group.
 *
 * @param mcpwm_group_id Group ID
 * @param timer MCPWM timer handler
 * @param operators MCPWM operator handler
 */
static void connect_timer_operator(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer, mcpwm_oper_handle_t* operator)
{
    ESP_LOGI(TAG, "Connect timers and operators with each other for group %d", mcpwm_group_id);
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(*operator, timer));
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
static void comparator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operator, mcpwm_cmpr_handle_t *comparator)
{
    ESP_LOGI(TAG, "Create comparator for second operator for group %d", mcpwm_group_id);
    // Configuration to use comparator with the overflow and underflow signals. Software sync disable.
    mcpwm_comparator_config_t compare_config = {
        .flags.update_cmp_on_tez = true,
        .flags.update_cmp_on_tep = true,
        .flags.update_cmp_on_sync = false,
    };

    // Create comparator in the operator
    ESP_ERROR_CHECK(mcpwm_new_comparator(*operator, &compare_config, comparator));
    ESP_LOGI(TAG, "Set comparator value: %d", COMPARE_VALUE_MIN);
    // Set the comparator value. (More details in the brief)
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(*comparator, COMPARE_VALUE_MIN));
}

/**
 * @brief Set comparator value during running
 *
 *
 * @param comparator Comparator handler
 * @param compare_value Desired compare value
 */
static void comparator_set_compare_value(mcpwm_cmpr_handle_t comparator, uint32_t compare_value)
{
    uint32_t calculated_value = (compare_value >= COMPARE_VALUE_MAX) ? COMPARE_VALUE_MAX : compare_value;
    calculated_value = (compare_value <= COMPARE_VALUE_MIN) ? COMPARE_VALUE_MIN : compare_value;

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, calculated_value));
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
static void generator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operator, mcpwm_gen_handle_t *generator, const uint8_t gen_gpio, mcpwm_cmpr_handle_t comparator)
{
    ESP_LOGI(TAG, "Create generators for group %d", mcpwm_group_id);
    mcpwm_generator_config_t gen_config = {};
    gen_config.gen_gpio_num = gen_gpio;
    ESP_ERROR_CHECK(mcpwm_new_generator(*operator, &gen_config, generator));
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

//*****************************************************************************
// Static functions END
//*****************************************************************************

void peltier_control_init_cooling()
{
    // ********************* Timer initialization *********************
    // Two peltier coolers are handled in one board.
    // One timer for peltier1 and one for peltier2
    mcpwm_timer_handle_t timer_group1;
    timer_init(PELTIER_MCPWM_GROUP_0, &timer_group1);
    mcpwm_timer_handle_t timer_group2;
    timer_init(PELTIER_MCPWM_GROUP_1, &timer_group2);

    // ********************* Operator initialization *********************
    mcpwm_oper_handle_t operator1;
    operator_init(PELTIER_MCPWM_GROUP_0, &operator1);
    mcpwm_oper_handle_t operator2;
    operator_init(PELTIER_MCPWM_GROUP_1, &operator2);

    // ********************* Timer and operator connection *********************
    connect_timer_operator(PELTIER_MCPWM_GROUP_0, timer_group1, &operator1);
    connect_timer_operator(PELTIER_MCPWM_GROUP_1, timer_group2, &operator2);

    // ********************* Comparator initialization *********************
    comparator_init(PELTIER_MCPWM_GROUP_0, &operator1, &comparator1);
    comparator_init(PELTIER_MCPWM_GROUP_1, &operator2, &comparator2);

    // ********************* GPIO initialization *********************
    const uint8_t gen_gpio1 = PELTIER1_COOLIN_PIN;
    generator_init(PELTIER_MCPWM_GROUP_0, &operator1, &peltier1_cooling_generator, gen_gpio1, comparator1);
    const uint8_t gen_gpio2 = PELTIER2_COOLIN_PIN;
    generator_init(PELTIER_MCPWM_GROUP_1, &operator2, &peltier2_cooling_generator, gen_gpio2, comparator2);

    // ********************* GPIO initialization *********************
    peltier_control_disable_cooling();

    ESP_LOGI(TAG, "Set generator actions on timer and compare event");
    gen_action_config(peltier1_cooling_generator, comparator1);
    gen_action_config(peltier2_cooling_generator, comparator2);

    // ********************* Start timer *********************
    start_timer(PELTIER_MCPWM_GROUP_0, timer_group1);
    start_timer(PELTIER_MCPWM_GROUP_1, timer_group2);
}

void peltier_control_enable_cooling()
{
    ESP_LOGI(TAG, "Enable output for cooling!");
    // remove the force level on the generator, so that we can see the PWM again
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(peltier1_cooling_generator, -1, true));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(peltier2_cooling_generator, -1, true));
}

void peltier_control_disable_cooling()
{
    ESP_LOGI(TAG, "Disable output for cooling!");
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(peltier1_cooling_generator, 0, true));
    ESP_ERROR_CHECK(mcpwm_generator_set_force_level(peltier2_cooling_generator, 0, true));
}

void peltier_control_task(void *pvParameters)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    uint32_t peltier1_desired_temp = 30;
    uint32_t peltier2_desired_temp = 30;

    uint32_t peltier1_current_temp;
    uint32_t peltier2_current_temp;

    uint32_t pid1_output;
    uint32_t pid2_output;

    peltier_control_init_cooling();
    peltier_control_enable_cooling();

#if USE_PID_CONTROLLER
    pid_controller_t peltier1_pid = {
        .Kp = -0.2,
        .Kd = 0,
        .Ki = -2,
        .tau = 0,
        .limitMin = COMPARE_VALUE_MIN,
        .limitMax = COMPARE_VALUE_MAX,
        .limitIntMin = -450,
        .limitIntMax = 450,
        .sampleTime = 1 // 1 second
    };

    pid_controller_t peltier2_pid = {
        .Kp = -0.2,
        .Kd = 0,
        .Ki = -2,
        .tau = 0,
        .limitMin = COMPARE_VALUE_MIN,
        .limitMax = COMPARE_VALUE_MAX,
        .limitIntMin = -1 * COMPARE_VALUE_MAX / 2,
        .limitIntMax = COMPARE_VALUE_MAX / 2,
        .sampleTime = 1 // 1 second
    };

    pid_controller_init(&peltier1_pid);
    pid_controller_init(&peltier2_pid);
#else
    int32_t peltier1_diff, peltier2_diff;
#endif

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 1,
        .pull_up_en = 0,
    };

    io_conf.pin_bit_mask = (1ULL << PELTIER1_HEATING_PIN);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << PELTIER2_HEATING_PIN);
    gpio_config(&io_conf);

    for (;;)
    {
        esp_log_level_set(TAG, ESP_LOG_INFO);
        if (xQueueReceive(peltier1_desired_temp_queue, &peltier1_desired_temp, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            pid_controller_init(&peltier1_pid);
        }
        if (xQueueReceive(peltier2_desired_temp_queue, &peltier2_desired_temp, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            pid_controller_init(&peltier2_pid);
        }

        peltier1_current_temp = laser_module_adc_read_temp1();
        peltier2_current_temp = laser_module_adc_read_temp2();
        ESP_LOGI(TAG, "Laser1 temp: %d | Laser2 temp: %d", peltier1_current_temp, peltier2_current_temp);

#if USE_PID_CONTROLLER
        pid1_output = pid_controller_update_peltier(&peltier1_pid, peltier1_desired_temp, peltier1_current_temp);
        comparator_set_compare_value(comparator1, pid1_output);
        ESP_LOGD(TAG, "Peltier1 comp. value: %d", pid1_output);
        
        pid2_output = pid_controller_update_peltier(&peltier2_pid, peltier2_desired_temp, peltier2_current_temp);
        comparator_set_compare_value(comparator2, pid2_output);
        ESP_LOGD(TAG, "Peltier2 comp value %d", pid2_output);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait ~1 sec
#else
        peltier1_diff = peltier1_current_temp - peltier1_desired_temp;
        peltier2_diff = peltier2_current_temp - peltier2_desired_temp;

        if (peltier1_diff >= 5)
        {
            comparator_set_compare_value(comparator1, COMPARE_VALUE_MAX / 2);
        }
        else if (peltier1_diff <= - 5)
        {
            comparator_set_compare_value(comparator1, COMPARE_VALUE_MIN);
        }

        if (peltier2_diff >= 5)
        {
            comparator_set_compare_value(comparator2, COMPARE_VALUE_MAX / 2);
        }
        else if (peltier2_diff <= -5)
        {
            comparator_set_compare_value(comparator2, COMPARE_VALUE_MIN);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
    }
}
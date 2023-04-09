#include "common_gpio.h"
#include "voa_control.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define TIMER_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define TIMER_PERIOD 1000           // 1000 ticks, 1ms
#define COMPARE_VALUE TIMER_PERIOD / 4

static const char *TAG = "VOA_CONTROL";
extern QueueHandle_t voa_attenuation_queue;

static mcpwm_gen_handle_t fwd_generators[2];
static mcpwm_gen_handle_t rev_generators[2];

typedef enum voa_mcpwm_group_id {
    VOA_MCPWM_GROUP_0 = 0,
    VOA_MCPWM_GROUP_1,
    VOA_MCPWM_GROUP_MAX
} voa_mcpwm_group_id_t;

static void gen_action_config(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t comp)
{
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gena,
                                                               MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                                                               MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
                                                               MCPWM_GEN_TIMER_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
                                                                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comp, MCPWM_GEN_ACTION_LOW),
                                                                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, comp, MCPWM_GEN_ACTION_HIGH),
                                                                 MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
}

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

static void operator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators)
{
    ESP_LOGI(TAG, "Create operators for group %d", mcpwm_group_id);
    mcpwm_operator_config_t operator_config = {
        .group_id = mcpwm_group_id, // operator should be in the same group of the above timers
    };
    for (int i = 0; i < 2; ++i)
    {
        ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &operators[i]));
    }
}

static void connect_timer_operator(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer, mcpwm_oper_handle_t *operators)
{
    ESP_LOGI(TAG, "Connect timers and operators with each other for group %d", mcpwm_group_id);
    for (int i = 0; i < 2; i++)
    {
        ESP_ERROR_CHECK(mcpwm_operator_connect_timer(operators[i], timer));
    }
}

static void comparator_init(uint8_t mcpwm_group_id, mcpwm_oper_handle_t *operators, mcpwm_cmpr_handle_t *comparator)
{
    ESP_LOGI(TAG, "Create comparator for second operator for group %d", mcpwm_group_id);
    mcpwm_comparator_config_t compare_config = {
        .flags.update_cmp_on_tez = true,
        .flags.update_cmp_on_tep = true,
        .flags.update_cmp_on_sync = false,
    };

    ESP_ERROR_CHECK(mcpwm_new_comparator(operators[1], &compare_config, comparator));
    ESP_LOGI(TAG, "Set comparator value: %d", COMPARE_VALUE);
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(*comparator, COMPARE_VALUE));
}

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

static void start_timer(uint8_t mcpwm_group_id, mcpwm_timer_handle_t timer)
{
    ESP_LOGI(TAG, "Start timer for group %d", mcpwm_group_id);
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

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

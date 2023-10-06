#include <math.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"

#include "driver/dac_oneshot.h"
#include "esp_log.h"

dac_oneshot_handle_t laser1_dac_handle;
dac_oneshot_handle_t laser2_dac_handle;

void laser_module_dac_init()
{
    /* DAC oneshot init */
    dac_oneshot_config_t chan0_cfg = {
        // TODO: Put this into the common.
        .chan_id = DAC_CHAN_1, // GPIO26
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan0_cfg, &laser1_dac_handle));

    dac_oneshot_config_t chan1_cfg = {
        .chan_id = DAC_CHAN_0, // GPIO25
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&chan1_cfg, &laser2_dac_handle));
}

void laser_module_dac_task()
{
    for (;;)
    {
        /* code */
    }
    
}
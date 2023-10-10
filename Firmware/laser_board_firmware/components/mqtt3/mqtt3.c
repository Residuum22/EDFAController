#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt3.h"

#include "cJSON.h"

#include "led_indicator.h"
#include "led_indicator_blink_default.h"
#include "led_indicator_laser.h"

static const char *TAG = "MQTT_EXAMPLE";

static cJSON *settings_json;

static cJSON *laser1_json;
static cJSON *laser2_json;

extern QueueHandle_t peltier1_desired_temp_queue, peltier2_desired_temp_queue;
extern QueueHandle_t laser1_enable_queue, laser2_enable_queue;
extern QueueHandle_t laser1_desired_current_queue, laser2_desired_current_queue;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        laser_indicator_set_state(BLINK_MQTT_CONNECTED);
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // Subscribe to Laser Module Set channal according to the LASER number
        char laser_module_number_str[2];
        itoa(CONFIG_LASER_MODULE_NUMBER, laser_module_number_str, 10);
        char laser_module_set_topic[21] = "/laser_module_set/";
        strcat(laser_module_set_topic, laser_module_number_str);
        ESP_LOGI(TAG, "Subscribe to the \"%s\" topic!", laser_module_set_topic);

        msg_id = esp_mqtt_client_subscribe(client, laser_module_set_topic, 2);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        laser_indicator_set_state(BLINK_MQTT_CONNECTING);
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        if (settings_json != NULL)
            cJSON_Delete(settings_json);
        settings_json = cJSON_Parse(event->data);
        // uint8_t report_interval = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(settings_json, "report_interval"));
#if CONFIG_LASER_MODULE_NUMBER == 1
        laser1_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_976_1");
        laser2_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_976_2");
#else
        laser1_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_1480_1");
        laser2_json = cJSON_GetObjectItemCaseSensitive(settings_json, "laser_1480_2");
#endif
        // Parse enable's
        cJSON *laser1_enable_json = cJSON_GetObjectItemCaseSensitive(laser1_json, "enabled");
        bool laser1_enabled = cJSON_IsTrue(laser1_enable_json) == cJSON_True;
        cJSON *laser2_enable_json = cJSON_GetObjectItemCaseSensitive(laser2_json, "enabled");
        bool laser2_enabled = cJSON_IsTrue(laser2_enable_json) == cJSON_True;
        ESP_LOGI(TAG, "Enable/Disable| Laser1: %d, Laser2: %d", laser1_enabled, laser2_enabled);
        xQueueSend(laser1_enable_queue, &laser1_enabled, pdMS_TO_TICKS(10));
        xQueueSend(laser2_enable_queue, &laser2_enabled, pdMS_TO_TICKS(10));

        // Parse desired temperature
        uint8_t laser1_desired_temp = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser1_json, "desired_temperature"));
        uint8_t laser2_desired_temp = (uint8_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser2_json, "desired_temperature"));
        ESP_LOGI(TAG, "Desired temperature| Laser1: %d, Laser2: %d", laser1_desired_temp, laser2_desired_temp);
        xQueueSend(peltier1_desired_temp_queue, &laser1_desired_temp, pdMS_TO_TICKS(10));
        xQueueSend(peltier2_desired_temp_queue, &laser2_desired_temp, pdMS_TO_TICKS(10));

        // Parse desired monitor diode current
        uint32_t laser1_desired_current = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser1_json, "desired_laser_current"));
        uint32_t laser2_desired_current = (uint32_t)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(laser2_json, "desired_laser_current"));
        ESP_LOGI(TAG, "Desired monitor diode| Laser1: %d, Laser2: %d", laser1_desired_current, laser2_desired_current);
        xQueueSend(laser1_desired_current_queue, &laser1_desired_current, pdMS_TO_TICKS(10));
        xQueueSend(laser2_desired_current_queue, &laser2_desired_current, pdMS_TO_TICKS(10));

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
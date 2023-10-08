/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The blink type with smaller index has the higher priority
 * eg. BLINK_FACTORY_RESET priority is higher than BLINK_UPDATING
 */
enum {
    BLINK_INDICATE_NONE,
    BLINK_INDICATE_ERROR,
    BLINK_WIFI_CONNECTING,
    BLINK_MQTT_CONNECTING,
    BLINK_MQTT_CONNECTED,
    BLINK_MAX,                     /*!< INVALID type */
};

extern const int DEFAULT_BLINK_LIST_NUM;
extern blink_step_t const *default_led_indicator_blink_lists[];

#ifdef __cplusplus
}
#endif

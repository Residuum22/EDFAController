/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/*  Private Funtions of protocol example common */

#pragma once

#include "esp_err.h"
#include "esp_wifi.h"
#include "sdkconfig.h"

void example_wifi_start(void);
void example_wifi_stop(void);
esp_err_t example_wifi_sta_do_connect(wifi_config_t wifi_config, bool wait);
esp_err_t example_wifi_sta_do_disconnect(void);
bool example_is_our_netif(const char *prefix, esp_netif_t *netif);
void example_print_all_netif_ips(const char *prefix);
void example_wifi_shutdown(void);
esp_err_t example_wifi_connect(void);
void example_ethernet_shutdown(void);
esp_err_t example_ethernet_connect(void);
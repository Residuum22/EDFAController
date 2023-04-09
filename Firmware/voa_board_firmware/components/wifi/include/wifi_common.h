#pragma once

#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_netif.h"

#define EXAMPLE_NETIF_DESC_STA "example_netif_sta"

/* Example default interface, prefer the ethernet one if running in example-test (CI) configuration */
#define EXAMPLE_INTERFACE get_example_netif_from_desc(EXAMPLE_NETIF_DESC_STA)
#define get_example_netif() get_example_netif_from_desc(EXAMPLE_NETIF_DESC_STA)

/**
 * @brief Configure Wi-Fi or Ethernet, connect, wait for IP
 *
 * This all-in-one helper function is used in protocols examples to
 * reduce the amount of boilerplate in the example.
 *
 * It is not intended to be used in real world applications.
 * See examples under examples/wifi/getting_started/ and examples/ethernet/
 * for more complete Wi-Fi or Ethernet initialization code.
 *
 * Read "Establishing Wi-Fi or Ethernet Connection" section in
 * examples/protocols/README.md for more information about this function.
 *
 * @return ESP_OK on successful connection
 */
esp_err_t wifi_connect(void);

/**
 * Counterpart to wifi_connect, de-initializes Wi-Fi or Ethernet
 */
esp_err_t example_disconnect(void);

/**
 * @brief Returns esp-netif pointer created by wifi_connect() described by
 * the supplied desc field
 *
 * @param desc Textual interface of created network interface, for example "sta"
 * indicate default WiFi station, "eth" default Ethernet interface.
 *
 */
esp_netif_t *get_example_netif_from_desc(const char *desc);
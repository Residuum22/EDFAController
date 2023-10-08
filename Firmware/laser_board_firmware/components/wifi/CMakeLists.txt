set(srcs "connect.c"
         "wifi_connect.c")

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_netif driver esp_wifi vfs led_indicator
    )

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
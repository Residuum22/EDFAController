idf_component_register(
    SRCS "mqtt3.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_netif nvs_flash driver vfs wifi mqtt log json led_indicator
    )
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

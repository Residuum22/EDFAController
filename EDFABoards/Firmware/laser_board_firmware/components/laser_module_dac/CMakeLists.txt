idf_component_register(
    SRCS "laser_module_dac.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES nvs_flash driver vfs log freertos soc main
    )
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
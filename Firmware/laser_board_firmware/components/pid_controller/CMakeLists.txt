idf_component_register(
    SRCS "pic_controller.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES nvs_flash driver vfs log freertos soc main laser_module_adc
    )
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
idf_component_register(SRC_DIRS "src" "src/MCPWM" "src/ADC"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_mcpwm esp_adc esp_driver_gpio esp_driver_gptimer esp_event esp_timer)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23 -fkeep-inline-functions)

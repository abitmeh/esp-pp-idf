file(GLOB_RECURSE SRC "*.cpp")

idf_component_register(SRCS ${SRC}
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_mcpwm esp_adc esp_driver_gpio esp_driver_gptimer)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23 -fkeep-inline-functions)


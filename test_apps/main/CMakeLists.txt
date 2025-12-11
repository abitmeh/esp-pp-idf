idf_component_register(SRC_DIRS "."
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES unity
                       WHOLE_ARCHIVE)

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++23 -DTESTING)

idf_component_register(
    INCLUDE_DIRS "inc"
)

add_prebuilt_library(volc_engine_rtc_lite "${CMAKE_CURRENT_SOURCE_DIR}/libs/${CONFIG_IDF_TARGET}/libVolcEngineRTCLite.a"
                    REQUIRES mbedtls espressif__zlib json lwip)
target_link_libraries(${COMPONENT_LIB} INTERFACE volc_engine_rtc_lite)

# Copyright (2025) Beijing Volcano Engine Technology Ltd.
# SPDX-License-Identifier: Apache-2.0

# ConversationalAI SDK ESP-IDF component configuration
# Converted from standard CMake to ESP-IDF component format

# Source files
set(VOLC_SRCS ${VOLC_CONV_AI_SRCS}
            "${CMAKE_CURRENT_LIST_DIR}/../volc_conv_ai/platforms/src/espressif/volc_platform.c"
)

set(VOLC_INCS ${VOLC_CONV_AI_INCS}
            "${CMAKE_CURRENT_LIST_DIR}/../volc_conv_ai/platforms/inc"
)

if(CONFIG_VOLC_RTC_MODE)
    message(STATUS "CONFIG_VOLC_RTC_MODE: ${CONFIG_VOLC_RTC_MODE}")
    list(APPEND VOLC_SRCS
        ${VOLC_CONV_AI_HIGH_QUALITY_SRCS}
    )
    list(APPEND VOLC_INCS
        ${VOLC_CONV_AI_HIGH_QUALITY_INCS}
    )
endif()

if(CONFIG_VOLC_WS_MODE)
    message(STATUS "CONFIG_VOLC_WS_MODE: ${CONFIG_VOLC_WS_MODE}")
    list(APPEND VOLC_SRCS
        ${VOLC_CONV_AI_LOW_LOAD_SRCS}
    )
    list(APPEND VOLC_INCS
        ${VOLC_CONV_AI_LOW_LOAD_INCS}
    )
endif()

idf_component_register(
    SRCS ${VOLC_SRCS}
    INCLUDE_DIRS ${VOLC_INCS}
    REQUIRES mbedtls  json lwip esp_netif
)

if(CONFIG_VOLC_RTC_MODE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ENABLE_RTC_MODE)
    
    # Add prebuilt library for RTC mode
    add_prebuilt_library(volc_engine_rtc_lite 
        "${CMAKE_CURRENT_LIST_DIR}/../volc_conv_ai/src/transports/high_quality/third_party/volc_rtc_engine_lite/libs/${CONFIG_IDF_TARGET}/libVolcEngineRTCLite.a"
        REQUIRES mbedtls espressif__zlib json lwip)
    target_link_libraries(${COMPONENT_LIB} INTERFACE volc_engine_rtc_lite)
endif()

if(CONFIG_VOLC_WS_MODE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ENABLE_WS_MODE)
endif()

# Compiler flags
target_compile_options(${COMPONENT_LIB} PRIVATE
    -w
    -Os
    -ffunction-sections
    -fdata-sections
)

# Export the library for use by other components
set(VOLC_CONV_AI_SDK_LIBRARIES ${COMPONENT_LIB} PARENT_SCOPE)
set(VOLC_CONV_AI_SDK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../inc PARENT_SCOPE)

cmake_minimum_required(VERSION 3.6.3)
project(VolcConvAIDemo LANGUAGES C)

set(ENABLE_WS_MODE ON)
add_definitions(-DENABLE_WS_MODE)

set(ENABLE_MBEDTLS ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/macos.cmake)

find_package(PkgConfig REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MBEDTLS_PREBUILT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../volc_conv_ai/third_party/prebuilt/mbedtls)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/util
)
add_compile_definitions(PLATFORM_MACOS)
link_directories(/opt/homebrew/lib)
include_directories(/opt/homebrew/include)
pkg_check_modules(PULSE REQUIRED libpulse)
pkg_check_modules(PULSE_SIMPLE REQUIRED libpulse-simple)

add_executable(
        volc_conv_ai_demo
        volc_conv_ai_demo.c
        util/volc_ringbuf.c
        util/cJSON.c
)

target_link_libraries(volc_conv_ai_demo
    volc_conv_ai_a
    ${PULSE_SIMPLE_LIBRARIES}
    pthread
)

install(TARGETS volc_conv_ai_demo DESTINATION ${CMAKE_BINARY_DIR}/bin)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/configs/conv_ai_config.json
        DESTINATION ${CMAKE_CURRENT_LIST_DIR}/build)

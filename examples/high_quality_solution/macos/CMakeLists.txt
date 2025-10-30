cmake_minimum_required(VERSION 3.6.3)
project(VolcConvAIDemo LANGUAGES C)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../volc_conv_ai/cmake/common.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../volc_conv_ai/cmake/macos.cmake)

set(ENABLE_RTC_MODE ON)
add_definitions(-DENABLE_RTC_MODE)

set(ENABLE_MBEDTLS ON)

find_package(PkgConfig REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

# 安装低负载解决方案的配置文件到指定目录
install(FILES ${CMAKE_CURRENT_LIST_DIR}/configs/conv_ai_config.json
        DESTINATION ${CMAKE_CURRENT_LIST_DIR}/build)

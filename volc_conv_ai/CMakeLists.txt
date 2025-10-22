cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/common.cmake)
if(ESP_PLATFORM)
    message(STATUS "ESP_PLATFORM is defined, including esp.cmake")
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/espressif.cmake)
    return()
endif()



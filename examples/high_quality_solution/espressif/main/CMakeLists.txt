# Copyright (2025) Beijing Volcano Engine Technology Ltd.
# SPDX-License-Identifier: Apache License 2.0

# include("../../../../application/framework/CMakeLists.txt")



file(GLOB  APP_SRC  "./echoear_app/*.c")

set(COMPONENT_SRCS "conv_ai_embedded_kit.c configuration_ap.c network.c" ${FRAMEWORK_SRCS} ${APP_SRC} ${SERVICE_SRCS})
if (CONFIG_VOLC_WS_MODE)
    add_definitions(-DENABLE_WS_MODE)
endif()

if (CONFIG_PRINT_TASK_INFO)
    add_definitions(-DPRINT_TASK_INFO)
endif()

# add_subdirectory("../../../../framework")

set(COMPONENT_EMBED_TXTFILES assets/wifi_configuration_ap.html)

register_component()


set(assets_root "${CMAKE_CURRENT_LIST_DIR}/../assets")

if(EXISTS "${assets_root}")
    spiffs_create_partition_assets(eaf "${assets_root}" FLASH_IN_PROJECT
        MMAP_FILE_SUPPORT_FORMAT ".eaf"
        IMPORT_INC_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    add_dependencies(${COMPONENT_LIB} assets_eaf_bin)
endif()

set(COMPONENT_ADD_INCLUDEDIRS . ${FRAMEWORK_INC} ${SERVICE_INC})

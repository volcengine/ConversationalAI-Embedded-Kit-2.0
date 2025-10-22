# Copyright (2025) Beijing Volcano Engine Technology Ltd.
# SPDX-License-Identifier: Apache License 2.0

set(COMPONENT_SRCS "conv_ai_embedded_kit.c pipeline.c configuration_ap.c network.c" )

if (CONFIG_PRINT_TASK_INFO)
    add_definitions(-DPRINT_TASK_INFO)
endif()

set(COMPONENT_ADD_INCLUDEDIRS .)
set(COMPONENT_EMBED_TXTFILES assets/wifi_configuration_ap.html)

register_component()

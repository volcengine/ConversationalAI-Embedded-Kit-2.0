set(COMMON_DIR "../../../../common")
set(COMMON_SRCS "${COMMON_DIR}/sdk/src/volc_conv_ai.c"
                "${COMMON_DIR}/sdk/src/base/volc_device_manager.c"
                "${COMMON_DIR}/sdk/src/util/volc_auth.c"
                "${COMMON_DIR}/sdk/src/util/volc_base64.c"
                "${COMMON_DIR}/sdk/src/util/volc_http.c"
                "${COMMON_DIR}/sdk/src/util/volc_json.c"
                "${COMMON_DIR}/third_party/mbedtls_port/tls_certificate.c"
                "${COMMON_DIR}/third_party/mbedtls_port/tls_client.c"
                "${COMMON_DIR}/third_party/webclient/src/webclient.c")

set(SRCS  "${COMMON_SRCS}"
          "../../../common/src/volc_rtc.c"
          "./src/volc_platform.c")
set(INCS "./inc" "../../../common/inc" "${COMMON_DIR}/sdk/inc" "${COMMON_DIR}/sdk/src" "${COMMON_DIR}/third_party/mbedtls_port" "${COMMON_DIR}/third_party/webclient/inc")

set(COMPONENT_SRCS ${SRCS})
set(COMPONENT_ADD_INCLUDEDIRS ${INCS})
set(COMPONENT_REQUIRES volc_engine_rtc_lite mbedtls json esp_netif lwip)

# add_definitions(-DENABLE_RTC_MODE)
if(CONFIG_VOLC_RTC_MODE)
    add_definitions(-DENABLE_RTC_MODE)
endif()

register_component()

cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(SW_SERVICES_DIR "$ENV{XMOS_AIOT_SDK_PATH}/modules/rtos/sw_services")

set(WIFI_MANAGER_DIR "${SW_SERVICES_DIR}/wifi_manager")
set(DHCPD_DIR "${SW_SERVICES_DIR}/dhcpd")
set(DEVICE_CONTROL_DIR "${SW_SERVICES_DIR}/device_control")
set(FATFS_DIR "${SW_SERVICES_DIR}/fatfs")
set(HTTP_PARSER_DIR "${SW_SERVICES_DIR}/http")
set(JSON_PARSER_DIR "${SW_SERVICES_DIR}/json")
set(MQTT_DIR "${SW_SERVICES_DIR}/mqtt")
set(SNTPD_DIR "${SW_SERVICES_DIR}/sntpd")
set(TLS_SUPPORT_DIR "${SW_SERVICES_DIR}/tls_support")
set(TINYUSB_DIR "${SW_SERVICES_DIR}/usb")

#**********************
# Options
#**********************
option(USE_WIFI_MANAGER "Enable to use wifi manager" FALSE)
option(USE_DHCPD "Enable to use DHCP" FALSE)
option(USE_DEVICE_CONTROL "Enable to use Device Control" FALSE)
option(USE_FATFS "Enable to use FATFS filesystem" FALSE)
option(USE_HTTP_PARSER "Enable to use HTTP parser" FALSE)
option(USE_JSON_PARSER "Enable to use JSON parser" FALSE)
option(USE_MQTT "Enable to use MQTT" FALSE)
option(USE_SNTPD "Enable to use SNTPD" FALSE)
option(USE_TLS_SUPPORT "Enable to use TLS support" FALSE)
option(USE_CUSTOM_MBEDTLS_CONFIG "Enable to use provide an alternate mbedtls_config.h" FALSE)
option(USE_TINYUSB "Enable to use TinyUSB" FALSE)

#********************************
# Gather wifi manager sources
#********************************
set(THIS_LIB WIFI_MANAGER)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "-Os")

	file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

    if(${${THIS_LIB}_FLAGS})
       set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/api"
	)

    add_compile_definitions(
        USE_WIFI_MANAGER=1
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather DHCPD sources
#********************************
set(THIS_LIB DHCPD)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "-Os")

	file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")

    if(${${THIS_LIB}_FLAGS})
       set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/api"
	)

    add_compile_definitions(
        USE_DHCPD=1
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather Device Control sources
#********************************
set(THIS_LIB DEVICE_CONTROL)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "-Os")

	file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/src/*.c")

    if(${${THIS_LIB}_FLAGS})
       set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/api"
	)

    add_compile_definitions(
        USE_DEVICE_CONTROL=1
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather FATFS sources
#********************************
set(THIS_LIB FATFS)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "-Os")

	file(GLOB_RECURSE ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/*.c")
	list(APPEND ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/thirdparty/src/ff.c")
	list(APPEND ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/thirdparty/src/ffunicode.c")

    if(${${THIS_LIB}_FLAGS})
        set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}"
	    "${${THIS_LIB}_DIR}/thirdparty/api"
	)

    add_compile_definitions(
        USE_FATFS=1
    )
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather HTTP parser sources
#********************************
set(THIS_LIB HTTP_PARSER)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "")

	set(${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/thirdparty/http-parser/http_parser.c")

    if(${${THIS_LIB}_FLAGS})
       set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/thirdparty/http-parser"
	)
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather JSON parser sources
#********************************
set(THIS_LIB JSON_PARSER)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "")

	set(${THIS_LIB}_SOURCES "")

    if(${${THIS_LIB}_FLAGS})
    	set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/thirdparty/jsmn"
	)
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather MQTT sources
#********************************
set(THIS_LIB MQTT)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "")

	set(${THIS_LIB}_SOURCES
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectClient.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTDeserializePublish.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTFormat.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSerializePublish.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeClient.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeClient.c"
        #"${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnectServer.c"
        #"${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTSubscribeServer.c"
        #"${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src/MQTTUnsubscribeServer.c"
        "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c"
        "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/MQTT${RTOS_CMAKE_RTOS}.c"
    )

    if(${${THIS_LIB}_FLAGS})
       set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTPacket/src"
	    "${${THIS_LIB}_DIR}/thirdparty/paho.mqtt.embedded-c/MQTTClient-C/src"
	    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}"
	)
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather SNTPD sources
#********************************
set(THIS_LIB SNTPD)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "")

	set(${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/sntpd.c")

    if(${${THIS_LIB}_FLAGS})
    	set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
	    "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}"
	)
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather TLS support sources
#********************************
set(THIS_LIB TLS_SUPPORT)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "")

	set(${THIS_LIB}_SOURCES
        "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/mbedtls_support.c"
        "${${THIS_LIB}_DIR}/thirdparty/port/mbedtls/${RTOS_CMAKE_RTOS}/mbedtls_xcore_platform.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/aes.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/aesni.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/arc4.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/aria.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/asn1parse.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/asn1write.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/base64.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/bignum.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/blowfish.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/camellia.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ccm.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/chacha20.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/chachapoly.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/cipher.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/cipher_wrap.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/cmac.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ctr_drbg.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/des.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/dhm.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ecdh.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ecdsa.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ecjpake.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ecp.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ecp_curves.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/entropy.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/entropy_poll.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/error.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/gcm.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/havege.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/hkdf.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/hmac_drbg.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/md.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/md2.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/md4.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/md5.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/md_wrap.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/memory_buffer_alloc.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/nist_kw.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/oid.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/padlock.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pem.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pk.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pk_wrap.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pkcs12.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pkcs5.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pkparse.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pkwrite.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/platform.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/platform_util.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/poly1305.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ripemd160.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/rsa.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/rsa_internal.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/sha1.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/sha256.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/sha512.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/threading.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/timing.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/version.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/version_features.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/xtea.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/certs.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/pkcs11.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509_create.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509_crl.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509_crt.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509_csr.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509write_crt.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/x509write_csr.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/debug.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/net_sockets.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_cache.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_ciphersuites.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_cli.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_cookie.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_srv.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_ticket.c"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/library/ssl_tls.c"
    )

    if(${${THIS_LIB}_FLAGS})
       set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
        "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/api"
        "${${THIS_LIB}_DIR}/thirdparty/mbedtls/include"
        "${${THIS_LIB}_DIR}/thirdparty/port/mbedtls/${RTOS_CMAKE_RTOS}"
	)

    if(NOT ${USE_CUSTOM_MBEDTLS_CONFIG})
        add_compile_definitions(
            MBEDTLS_CONFIG_FILE=\"mbedtls_xcore_default_config.h\"
        )

        message(WARNING "${COLOR_YELLOW}Using default XCore mbed TLS configuration.\nTo use a different configuration, enable cmake option USE_CUSTOM_MBEDTLS_CONFIG, provide an mbedtls_config.h file with options required for your application, and add compiler definition MBEDTLS_CONFIG_FILE=\\\"path_to_your_file/mbedtls_config.h\\\"${COLOR_RESET}")
    endif()
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#********************************
# Gather TinyUSB sources
#********************************
set(THIS_LIB TINYUSB)
if(${USE_${THIS_LIB}})
	set(${THIS_LIB}_FLAGS "")

    file(GLOB_RECURSE ROOT_SOURCES "${${THIS_LIB}_DIR}/thirdparty/tinyusb/src/tusb.c")
    file(GLOB_RECURSE CLASS_SOURCES "${${THIS_LIB}_DIR}/thirdparty/tinyusb/src/class/*/*.c")
    file(GLOB_RECURSE COMMON_SOURCES "${${THIS_LIB}_DIR}/thirdparty/tinyusb/src/common/*.c")
    file(GLOB_RECURSE DEVICE_SOURCES "${${THIS_LIB}_DIR}/thirdparty/tinyusb/src/device/*.c")

    set(${THIS_LIB}_SOURCES  ${ROOT_SOURCES}
                             ${CLASS_SOURCES}
                             ${COMMON_SOURCES}
                             ${DEVICE_SOURCES}
                             ${HOST_SOURCES}
                             "${${THIS_LIB}_DIR}/${RTOS_CMAKE_RTOS}/usb_support.c"
                             "${${THIS_LIB}_DIR}/msc/msc_disk_manager.c"
                             "${${THIS_LIB}_DIR}/msc/msc_ramdisk.c"
                             "${${THIS_LIB}_DIR}/portable/dcd_xcore.c")

    if(USE_RTOS_QSPI_FLASH_DRIVER)
        list(APPEND ${THIS_LIB}_SOURCES "${${THIS_LIB}_DIR}/msc/msc_flashdisk.c")
    endif()

    if(${${THIS_LIB}_FLAGS})
    	set_source_files_properties(${${THIS_LIB}_SOURCES} PROPERTIES COMPILE_FLAGS ${${THIS_LIB}_FLAGS})
    endif()

	set(${THIS_LIB}_INCLUDES
        "${${THIS_LIB}_DIR}/api"
	    "${${THIS_LIB}_DIR}/msc"
	    "${${THIS_LIB}_DIR}/portable"
	    "${${THIS_LIB}_DIR}/thirdparty/tinyusb/src"
	)
    message("${COLOR_GREEN}Adding ${THIS_LIB}...${COLOR_RESET}")
endif()
unset(THIS_LIB)

#**********************
# set user variables
#**********************
set(SW_SERVICES_SOURCES
    ${DEVICE_CONTROL_SOURCES}
    ${FATFS_SOURCES}
    ${JSON_PARSER_SOURCES}
    ${TINYUSB_SOURCES}
)

set(SW_SERVICES_INCLUDES
    ${DEVICE_CONTROL_INCLUDES}
    ${FATFS_INCLUDES}
    ${JSON_PARSER_INCLUDES}
    ${TINYUSB_INCLUDES}
)

list(REMOVE_DUPLICATES SW_SERVICES_SOURCES)
list(REMOVE_DUPLICATES SW_SERVICES_INCLUDES)

set(SW_SERVICES_NETWORKING_SOURCES
    ${WIFI_MANAGER_SOURCES}
    ${DHCPD_SOURCES}
    ${HTTP_PARSER_SOURCES}
    ${MQTT_SOURCES}
    ${SNTPD_SOURCES}
    ${TLS_SUPPORT_SOURCES}
)

set(SW_SERVICES_NETWORKING_INCLUDES
    ${WIFI_MANAGER_INCLUDES}
    ${DHCPD_INCLUDES}
    ${HTTP_PARSER_INCLUDES}
    ${MQTT_INCLUDES}
    ${SNTPD_INCLUDES}
    ${TLS_SUPPORT_INCLUDES}
)

list(REMOVE_DUPLICATES SW_SERVICES_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES SW_SERVICES_NETWORKING_INCLUDES)

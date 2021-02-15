cmake_minimum_required(VERSION 3.14)

#**********************
# Paths
#**********************
set(TINYUSB_DEMO_PATH "${CMAKE_SOURCE_DIR}/src/tinyusb_demos")

set(AUDIO_TEST_PATH "${TINYUSB_DEMO_PATH}/audio_test/src")
set(CDC_DUAL_PORTS_TEST_PATH "${TINYUSB_DEMO_PATH}/cdc_dual_ports/src")
set(CDC_MSC_TEST_PATH "${TINYUSB_DEMO_PATH}/cdc_msc/src")
set(CDC_MSC_FREERTOS_TEST_PATH "${TINYUSB_DEMO_PATH}/cdc_msc_freertos/src")
set(DFU_RUNTIME_TEST_PATH "${TINYUSB_DEMO_PATH}/dfu_runtime/src")
set(DYNAMIC_CONFIGURATION_TEST_PATH "${TINYUSB_DEMO_PATH}/dynamic_configuration/src")
set(HID_COMPOSITE_TEST_PATH "${TINYUSB_DEMO_PATH}/hid_composite/src")
set(HID_GENERIC_INOUT_TEST_PATH "${TINYUSB_DEMO_PATH}/hid_generic_inout/src")
set(HID_MULTIPLE_INTERFACE_TEST_PATH "${TINYUSB_DEMO_PATH}/hid_multiple_interface/src")
set(MIDI_TEST_PATH "${TINYUSB_DEMO_PATH}/midi_test/src")
set(MSC_DUAL_LUN_PATH "${TINYUSB_DEMO_PATH}/msc_dual_lun/src")
set(NET_LWIP_WEBSERVER_PATH "${TINYUSB_DEMO_PATH}/net_lwip_webserver/src")
set(UAC2_HEADSET_PATH "${TINYUSB_DEMO_PATH}/uac2_headset/src")
set(USBTMC_PATH "${TINYUSB_DEMO_PATH}/usbtmc/src")
set(WEBUSB_SERIAL "${TINYUSB_DEMO_PATH}/webusb_serial/src")

#**********************
# Options
#**********************
option(USE_TINYUSB_DEMO_AUDIO_TEST                  "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_CDC_DUAL_PORTS_TEST         "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_CDC_MSC_TEST                "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_CDC_MSC_FREERTOS_TEST       "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_DFU_RUNTIME_TEST            "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_DYNAMIC_CONFIGURATION_TEST  "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_HID_COMPOSITE_TEST          "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_HID_GENERIC_INOUT_TEST      "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_HID_MULTIPLE_INTERFACE_TEST "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_MIDI_TEST                   "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_MSC_DUAL_LUN                "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_NET_LWIP_WEBSERVER          "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_UAC2_HEADSET                "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_USBTMC                      "Enable to use TinyUSB demo" FALSE)
option(USE_TINYUSB_DEMO_WEBUSB_SERIAL               "Enable to use TinyUSB demo" FALSE)


if(USE_TINYUSB_DEMO_AUDIO_TEST)
    set(DEMO_PATH ${AUDIO_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_CDC_DUAL_PORTS_TEST)
    set(DEMO_PATH ${CDC_DUAL_PORTS_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_CDC_MSC_TEST)
    set(DEMO_PATH ${CDC_MSC_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_CDC_MSC_FREERTOS_TEST)
    set(DEMO_PATH ${CDC_MSC_FREERTOS_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_DFU_RUNTIME_TEST)
    set(DEMO_PATH ${DFU_RUNTIME_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_DYNAMIC_CONFIGURATION_TEST)
    set(DEMO_PATH ${DYNAMIC_CONFIGURATION_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_HID_COMPOSITE_TEST)
    set(DEMO_PATH ${HID_COMPOSITE_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_HID_GENERIC_INOUT_TEST)
    set(DEMO_PATH ${HID_GENERIC_INOUT_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_HID_MULTIPLE_INTERFACE_TEST)
    set(DEMO_PATH ${HID_MULTIPLE_INTERFACE_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_MIDI_TEST)
    set(DEMO_PATH ${MIDI_TEST_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_MSC_DUAL_LUN)
    set(DEMO_PATH ${MSC_DUAL_LUN_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_NET_LWIP_WEBSERVER)
    set(DEMO_PATH ${NET_LWIP_WEBSERVER_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_UAC2_HEADSET)
    set(DEMO_PATH ${UAC2_HEADSET_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_USBTMC)
    set(DEMO_PATH ${USBTMC_PATH})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

if(USE_TINYUSB_DEMO_WEBUSB_SERIAL)
    set(DEMO_PATH ${WEBUSB_SERIAL})

    set(TINYUSB_DEMO_SOURCES
        "${DEMO_PATH}/demo_main.c"
        "${DEMO_PATH}/usb_descriptors.c"
    )

    set(TINYUSB_DEMO_INCLUDES
        ${DEMO_PATH}
    )
endif()

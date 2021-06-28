.. _sdk-build-configurations-label:

####################
Build Configurations
####################

*************************
HIL Configuration Options
*************************

Interfaces required by the XCore Hardware Imitation Layer (HIL) can be included or excluded with the following options.

.. list-table:: HIL Configuration Options
    :widths: 25 10 50
    :header-rows: 1

    * - Option
      - Default
      - Description
    * - USE_I2C_HIL
      - TRUE
      - Enable to include I2C HIL
    * - USE_I2S_HIL
      - TRUE
      - Enable to include I2S HIL
    * - USE_I2C_HIL
      - TRUE
      - Enable to include I2C HIL
    * - USE_MIC_ARRAY_HIL
      - TRUE
      - Enable to include microphone array HIL
    * - USE_SPI_HIL
      - TRUE
      - Enable to include SPI HIL
    * - USE_QSPI_IO_HIL
      - TRUE
      - Enable to include QSPI HIL
    * - USE_XUD_HIL
      - FALSE
      - Enable to include XUD HIL


*********************************
RTOS Driver Configuration Options
*********************************

RTOS drivers can be included or excluded with the following options.

.. list-table:: RTOS Driver Configuration Options
    :widths: 25 10 50
    :header-rows: 1

    * - Option
      - Default
      - Description
    * - USE_RTOS_GPIO_DRIVER
      - TRUE
      - Enable to include RTOS GPIO driver
    * - USE_RTOS_I2C_DRIVER
      - TRUE
      - Enable to include RTOS I2C driver
    * - USE_RTOS_I2S_DRIVER
      - TRUE
      - Enable to include RTOS I2S driver
    * - USE_RTOS_SPI_DRIVER
      - TRUE
      - Enable to include RTOS SPI driver
    * - USE_RTOS_QSPI_FLASH_DRIVER
      - TRUE
      - Enable to include RTOS QSPI flash driver
    * - USE_RTOS_MIC_ARRAY_DRIVER
      - TRUE
      - Enable to include RTOS microphone array driver
    * - USE_RTOS_RPC_DRIVER
      - TRUE
      - Enable to include RTOS intertile remote procedure call driver
    * - USE_RTOS_INTERTILE_DRIVER
      - TRUE
      - Enable to include RTOS intertile communication driver
    * - USE_RTOS_USB_DRIVER
      - FALSE
      - Enable to include RTOS USB driver
    * - USE_RTOS_SWMEM_DRIVER
      - TRUE
      - Enable to include RTOS SWMem driver
    * - USE_RTOS_WIFI_DRIVER
      - TRUE
      - Enable to include RTOS WiFi driver
    * - USE_RTOS_TRACE_DRIVER
      - TRUE
      - Enable to include RTOS trace driver

***********************************
Miscellaneous Configuration Options
***********************************

Several middleware components and libraries can be included or excluded with the following options.

.. list-table:: Miscellaneous Configuration Options
    :widths: 25 10 50
    :header-rows: 1

    * - Option
      - Default
      - Description
    * - USE_MULTITILE_SUPPORT
      - TRUE
      - Enable multitile support
    * - USE_TINYUSB
      - FALSE
      - Enable to use TinyUSB
    * - USE_DISK_MANAGER_TUSB
      - FALSE
      - Enable to use RAM and Flash disk manager
    * - USE_DEVICE_CONTROL
      - FALSE
      - Enable to use Device Control
    * - USE_FATFS
      - FALSE
      - Enable to use FATFS filesystem
    * - USE_DISPATCHER
      - FALSE
      - Enable to use Dispatcher
    * - USE_LIB_RANDOM
      - TRUE
      - Enable to include lib_random
    * - USE_LIB_XS3_MATH
      - FALSE
      - Enable to include lib_xs3_math

**********************************
AI Inference Configuration Options
**********************************

Several AI inference libraries can be included or excluded with the following options.

.. list-table:: AI Inference Configuration Options
    :widths: 25 10 50
    :header-rows: 1

    * - Option
      - Default
      - Description
    * - USE_AIF
      - FALSE
      - Enable to include the AI model Inference Framework libraries
    * - USE_DEVICE_MEMORY_SUPPORT
      - FALSE
      - Enable to include support for models stored in ExtMem or SwMem device memory

*************************
IoT Configuration Options
*************************

Several IoT libraries can be included or excluded with the following options.

.. list-table:: IoT Configuration Options
    :widths: 25 10 50
    :header-rows: 1

    * - Option
      - Default
      - Description
    * - USE_WIFI_MANAGER
      - FALSE
      - Enable use WiFi manager
    * - USE_DHCPD
      - FALSE
      - Enable to use DHCP
    * - USE_HTTP_CORE
      - FALSE
      - Enable to use HTTP client and parser
    * - USE_HTTP_PARSER
      - FALSE
      - Enable to use HTTP parser
    * - USE_JSON_PARSER
      - FALSE
      - Enable to to use JSON parser
    * - USE_MQTT
      - FALSE
      - Enable to use MQTT
    * - USE_SNTPD
      - FALSE
      - Enable to use SNTPD
    * - USE_TLS_SUPPORT
      - FALSE
      - Enable to use TLS
    * - USE_CUSTOM_MBEDTLS_CONFIG
      - FALSE
      - Enable to use an alternate mbedtls_config.h

************************************
Legacy Library Configuration Options
************************************

Several legacy libraries can be included or excluded with the following options.

.. list-table:: Legacy Library Configuration Options
    :widths: 25 10 50
    :header-rows: 1

    * - Option
      - Default
      - Description
    * - USE_LIB_DSP
      - TRUE
      - Enable to include lib_dsp
    * - USE_LIB_LOGGING
      - TRUE
      - Enable to include lib_logging
    * - USE_LEGACY_COMPAT
      - TRUE
      - Enable to include legacy compatibility layer for XMOS libraries

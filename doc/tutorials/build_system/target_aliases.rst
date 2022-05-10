.. _sdk-cmake-target-aliases:

##############
Target Aliases
##############

The following library target aliases can be used in your application.  An example of how to add aliases to your target link libraries is shown below:

.. code-block:: cmake

  target_link_libraries(my_app PUBLIC sdk::core sdk::rtos_freertos sdk::rtos_bsp::xcore_ai_explorer)

*******
General
*******

Several aliases are provided that specify a collection of libraries with similar functions.  These composite target libraries provide a concise alternative to specifying all the individual targets that are commonly required.

.. list-table:: Composite Target Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::core
      - All core libraries used my most XCore applications
    * - sdk::multitile_support
      - Libraries to support development on multiple tiles
    * - sdk::hil
      - All Hardware Imitation Layer (HIL) libraries

If you prefer, you can specify individual Hardware Imitation Layer (HIL) libraries.

.. list-table:: Individual Hardware Imitation Layer (HIL) Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::hil::lib_i2c
      - I2C library
    * - sdk::hil::lib_i2s
      - I2S library
    * - sdk::hil::lib_spi
      - SPI library
    * - sdk::hil::lib_qspi_io
      - QSPI library
    * - sdk::hil::lib_mic_array
      - Microphone Array library
    * - sdk::hil::lib_xud
      - XUD USB library
    * - sdk::hil::lib_l2_cache
      - L2 Cache library
    * - sdk::hil::lib_clock_control
      - Clock control library

The following libraries are also provided by the SDK.

.. list-table:: Additional Libraries and Modules
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::lib_src
      - Sample rate conversion API
    * - sdk::lib_xs3_math
      - Optimize math and DSP API
    * - sdk::utils
      - General utilities used by most applications

****
RTOS
****

Several aliases are provided that specify a collection of RTOS libraries with similar functions.  These composite target libraries provide a concise alternative to specifying all the individual targets that are commonly required.

.. list-table:: Composite RTOS Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::rtos_freertos
      - All core libraries used my most XCore FreeRTOS applications
    * - sdk::rtos::drivers
      - All RTOS Driver libraries
    * - sdk::rtos_usb
      - All libraries to support development with TinyUSB
    * - sdk::rtos::sw_services
      - All RTOS software service libraries
    * - sdk::rtos::iot
      - All IoT libraries
    * - sdk::rtos::wifi
      - All WiFi libraries

These board support libraries simplify development with a specific board.

.. list-table:: Board Support Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::rtos_bsp::xcore_ai_explorer
      - xcore.ai Explorer RTOS board support library
    * - sdk::rtos_bsp::xs200_micarray
      - XCore-200 Circular Mic Array RTOS board support library

If you prefer, you can specify individual RTOS driver libraries.

.. list-table:: Individual RTOS Driver Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::rtos::drivers::i2c
      - I2C RTOS driver library
    * - sdk::rtos::drivers::i2s
      - I2S RTOS driver library
    * - sdk::rtos::drivers::spi
      - SPI RTOS driver library
    * - sdk::rtos::drivers::qspi_io
      - QSPI RTOS driver library
    * - sdk::rtos::drivers::mic_array
      - Microphone Array RTOS driver library
    * - sdk::rtos::drivers::usb
      - USB RTOS driver library
    * - sdk::rtos::drivers::gpio
      - GPIO RTOS driver library
    * - sdk::rtos::drivers::l2_cache
      - L2 Cache RTOS driver library
    * - sdk::rtos::drivers::clock_control
      - Clock control RTOS driver library
    * - sdk::rtos::drivers::trace
      - Trace RTOS driver library
    * - sdk::rtos::drivers::swmem
      - SwMem RTOS driver library
    * - sdk::rtos::drivers::wifi
      - WiFi RTOS driver library
    * - sdk::rtos::drivers::intertile
      - Intertile RTOS driver library
    * - sdk::rtos::drivers::rpc
      - Remote procedure call RTOS driver library

If you prefer, you can specify individual software service libraries.

.. list-table:: Individual Software Service Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::rtos::sw_services::fatfs
      - FatFS library
    * - sdk::rtos::sw_services::usb
      - USB library
    * - sdk::rtos::sw_services::device_control
      - Device control library
    * - sdk::rtos::sw_services::usb_device_control
      - USB device control library
    * - sdk::rtos::sw_services::dispatcher
      - Dispatcher thread pool library
    * - sdk::rtos::sw_services::wifi_manager
      - WiFi manager library
    * - sdk::rtos::sw_services::tls_support
      - TLS library
    * - sdk::rtos::sw_services::dhcp
      - DHCP library
    * - sdk::rtos::sw_services::json
      - JSON library
    * - sdk::rtos::sw_services::http
      - HTTP library
    * - sdk::rtos::sw_services::sntpd
      - SNTP daemon library
    * - sdk::rtos::sw_services::mqtt
      - MQTT library

The following libraries for building host applications are also provided by the SDK.

.. list-table:: Host (x86) Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - sdk::rtos::sw_services::device_control_host_usb
      - Host USB device control library

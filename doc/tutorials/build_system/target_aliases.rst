.. _sdk-cmake-target-aliases:

##############
Target Aliases
##############

The following library target aliases can be used in your application `CMakeLists.txt`.  An example of how to add aliases to your target link libraries is shown below:

.. code-block:: cmake

  target_link_libraries(my_app PUBLIC core::general rtos::freertos sdk::rtos_bsp::xcore_ai_explorer)

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
    * - core::all
      - All core libraries
    * - io::all
      - All peripheral libraries
    * - rtos::freertos
      - All RTOS libraries


****
Core
****

If you prefer, you can specify individual core libraries.

.. list-table:: Core Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - core::clock_control
      - Clock control API
    * - core::utils
      - General utilities used by most applications
    * - core::xs3_math
      - Optimize math and DSP API
    * - core::general
      - All core libraries

***********
Peripherals
***********

If you prefer, you can specify individual peripheral libraries.

.. list-table:: Peripheral Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - io::i2c
      - I2C library
    * - io::spi
      - SPI library
    * - io::uart
      - UART library
    * - io::qspi_io
      - QSPI library
    * - io::xud
      - XUD USB library
    * - io::i2s
      - I2S library
    * - io::mic_array
      - Microphone Array library

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
    * - rtos::freertos
      - All libraries used my most xcore FreeRTOS applications
    * - rtos::drivers:all
      - All RTOS Driver libraries
    * - rtos::freertos_usb
      - All libraries to support development with TinyUSB
    * - rtos::sw_services::general
      - Most commonly used RTOS software service libraries
    * - rtos::iot
      - All IoT libraries
    * - rtos::wifi
      - All WiFi libraries

These board support libraries simplify development with a specific board.

.. list-table:: Board Support Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - rtos::bsp_config::xcore_ai_explorer
      - xcore.ai Explorer RTOS board support library

If you prefer, you can specify individual RTOS driver libraries.

.. list-table:: Individual RTOS Driver Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - rtos::drivers::uart
      - UART RTOS driver library
    * - rtos::drivers::i2c
      - I2C RTOS driver library
    * - rtos::drivers::i2s
      - I2S RTOS driver library
    * - rtos::drivers::spi
      - SPI RTOS driver library
    * - rtos::drivers::qspi_io
      - QSPI RTOS driver library
    * - rtos::drivers::mic_array
      - Microphone Array RTOS driver library
    * - rtos::drivers::usb
      - USB RTOS driver library
    * - rtos::drivers::gpio
      - GPIO RTOS driver library
    * - rtos::drivers::l2_cache
      - L2 Cache RTOS driver library
    * - rtos::drivers::clock_control
      - Clock control RTOS driver library
    * - rtos::drivers::trace
      - Trace RTOS driver library
    * - rtos::drivers::swmem
      - SwMem RTOS driver library
    * - rtos::drivers::wifi
      - WiFi RTOS driver library
    * - rtos::drivers::intertile
      - Intertile RTOS driver library
    * - rtos::drivers::rpc
      - Remote procedure call RTOS driver library

If you prefer, you can specify individual software service libraries.

.. list-table:: Individual Software Service Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - rtos::sw_services::fatfs
      - FatFS library
    * - rtos::sw_services::usb
      - USB library
    * - rtos::sw_services::device_control
      - Device control library
    * - rtos::sw_services::usb_device_control
      - USB device control library
    * - rtos::sw_services::wifi_manager
      - WiFi manager library
    * - rtos::sw_services::tls_support
      - TLS library
    * - rtos::sw_services::dhcp
      - DHCP library
    * - rtos::sw_services::json
      - JSON library
    * - rtos::sw_services::http
      - HTTP library
    * - rtos::sw_services::sntpd
      - SNTP daemon library
    * - rtos::sw_services::mqtt
      - MQTT library

The following libraries for building host applications are also provided by the SDK.

.. list-table:: Host (x86) Libraries
    :widths: 50 50
    :header-rows: 1
    :align: left

    * - Target
      - Description
    * - rtos::sw_services::device_control_host_usb
      - Host USB device control library

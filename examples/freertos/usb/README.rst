###
USB
###

This example application demonstrates how to use USB device classes.  The application build one of several `TinyUSB <https://docs.tinyusb.org/en/latest/>`__ demonstration applications.

*********************
Building the firmware
*********************

To build the firmware, run make, specifying the desired usb demo firmware.

Recipe options are:
    * example_freertos_usb_tusb_demo_audio_test
    * example_freertos_usb_tusb_demo_cdc_dual_ports
    * example_freertos_usb_tusb_demo_cdc_msc
    * example_freertos_usb_tusb_demo_dfu_runtime
    * example_freertos_usb_tusb_demo_hid_composite
    * example_freertos_usb_tusb_demo_hid_generic_inout
    * example_freertos_usb_tusb_demo_hid_multiple_interface
    * example_freertos_usb_tusb_demo_midi_test
    * example_freertos_usb_tusb_demo_msc_dual_lun
    * example_freertos_usb_tusb_demo_usbtmc
    * example_freertos_usb_tusb_demo_webusb_serial

Per example, to build the midi test demo, run the following commands in the xcore_sdk root folder:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_freertos_usb_tusb_demo_midi_test

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_freertos_usb_tusb_demo_midi_test


********************
Running the firmware
********************

To run the firmware, run make, specifying the desired usb demo firmware.

Recipe options are:
    * run_example_freertos_usb_tusb_demo_audio_test
    * run_example_freertos_usb_tusb_demo_cdc_dual_ports
    * run_example_freertos_usb_tusb_demo_cdc_msc
    * run_example_freertos_usb_tusb_demo_dfu_runtime
    * run_example_freertos_usb_tusb_demo_hid_composite
    * run_example_freertos_usb_tusb_demo_hid_generic_inout
    * run_example_freertos_usb_tusb_demo_hid_multiple_interface
    * run_example_freertos_usb_tusb_demo_midi_test
    * run_example_freertos_usb_tusb_demo_msc_dual_lun
    * run_example_freertos_usb_tusb_demo_usbtmc
    * run_example_freertos_usb_tusb_demo_webusb_serial


Per example, to run the midi test demo, run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_example_freertos_usb_tusb_demo_midi_test

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_usb_tusb_demo_midi_test

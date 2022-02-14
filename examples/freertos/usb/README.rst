###
USB
###

This example application demonstrates how to use USB device classes.  The application build one of several `TinyUSB <https://docs.tinyusb.org/en/latest/>`__ demonstration applications.

*********************************
Building the firmware
*********************************

To build the firmware, run make, specifying the desired usb demo firmware.

Recipe options are:
- usb_tusb_demo_audio_test
- usb_tusb_demo_cdc_dual_ports
- usb_tusb_demo_cdc_msc
- usb_tusb_demo_dfu_runtime
- usb_tusb_demo_hid_composite
- usb_tusb_demo_hid_generic_inout
- usb_tusb_demo_hid_multiple_interface
- usb_tusb_demo_midi_test
- usb_tusb_demo_msc_dual_lun
- usb_tusb_demo_usbtmc
- usb_tusb_demo_webusb_serial

Per example, to build the midi test demo, run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make usb_tusb_demo_midi_test

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake usb_tusb_demo_midi_test

*********************************
Running the firmware
*********************************
To run the firmware, run make, specifying the desired usb demo firmware.

Recipe options are:
- run_usb_tusb_demo_audio_test
- run_usb_tusb_demo_cdc_dual_ports
- run_usb_tusb_demo_cdc_msc
- run_usb_tusb_demo_dfu_runtime
- run_usb_tusb_demo_hid_composite
- run_usb_tusb_demo_hid_generic_inout
- run_usb_tusb_demo_hid_multiple_interface
- run_usb_tusb_demo_midi_test
- run_usb_tusb_demo_msc_dual_lun
- run_usb_tusb_demo_usbtmc
- run_usb_tusb_demo_webusb_serial
- run_vwv_bare_metal

Per example, to run the midi test demo, run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_usb_tusb_demo_midi_test

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake run_usb_tusb_demo_midi_test

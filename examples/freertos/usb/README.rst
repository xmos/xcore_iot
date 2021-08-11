###
USB
###

This example application demonstrates how to use USB device classes.  The application build one of several `TinyUSB <https://docs.tinyusb.org/en/latest/>`__ demonstration applications.  

*********************************
Building and running the firmware
*********************************

Run the following commands to build the firmware:

.. code-block:: console

    $ make
    $ make run

By default, the TinyUSB AUDIO_TEST application is built.  Use the `TINYUSB_DEMO` option to build one of the following supported demos.  

- AUDIO_TEST
- HID_COMPOSITE_TEST
- WEBUSB_SERIAL
- MIDI_TEST
- USBTMC
- CDC_MSC_TEST
- MSC_DUAL_LUN
- DFU_RUNTIME_TEST
- CDC_DUAL_PORTS_TEST
- HID_GENERIC_INOUT_TEST
- HID_MULTIPLE_INTERFACE_TEST

For example, to build the MIDI_TEST, run the following command:

.. code-block:: console

    $ make TINYUSB_DEMO=MIDI_TEST
    $ make run

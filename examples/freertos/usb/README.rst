###
USB
###

This example application demonstrates how to use USB device classes.  The application build one of several `TinyUSB <https://docs.tinyusb.org/en/latest/>`__ demonstration applications.  

*********************************
Building and running the firmware
*********************************

Run the following commands to build the firmware:

.. tab:: Linux and Mac

	.. code-block:: console

		$ cmake -B build -DBOARD=XCORE-AI-EXPLORER
		$ cd build
		$ make
		
.. tab:: Windows

	.. code-block:: XTC Tools CMD prompt

		> cmake -G "NMake Makefiles" -B build -DBOARD=XCORE-AI-EXPLORER
		> cd build
		> nmake
		
To run the example:

.. tab:: Linux and Mac

	.. code-block:: console

		$ xrun --xscope bin/usb.xe

.. tab:: Windows

	.. code-block:: XTC Tools CMD prompt

		> xrun --xscope bin\usb.xe

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

.. tab:: Linux and Mac

	.. code-block:: console

		$ cmake -B build -DBOARD=XCORE-AI-EXPLORER -DTINYUSB_DEMO_TO_USE=MIDI_TEST
		$ cd build
		$ make

.. tab:: Windows

	.. code-block:: XTC Tools CMD prompt

		> cmake -G "NMake Makefiles" -B build -DBOARD=XCORE-AI-EXPLORER -DTINYUSB_DEMO_TO_USE=MIDI_TEST
		> cd build
		> nmake
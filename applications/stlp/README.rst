============================
Avona Voice Reference Design
============================

This is the XMOS Avona voice reference design.

******************
Supported Hardware
******************

This example is supported on the XK_VOICE_L71 board.

*********************
Building the Firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make application_stlp

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake application_stlp

From the xcore_sdk build folder, create the filesystem and flash the device with the following command:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make flash_fs_application_stlp

.. tab:: Windows

    .. code-block:: console

        $ nmake flash_fs_application_stlp

********************
Running the Firmware
********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_application_stlp

.. tab:: Windows

    .. code-block:: console

        $ nmake run_application_stlp


********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make debug_application_stlp

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_application_stlp


********************
Running the Firmware With WAV Files
********************

This application supports USB audio input and output debug configuration.

To enable USB audio debug, add the following compile definitions:

.. tab:: WAV File Debug Additional Compile Definitions

    appconfUSB_ENABLED=1
    appconfMIC_SRC_DEFAULT=appconfMIC_SRC_USB
    appconfAEC_REF_DEFAULT=appconfAEC_REF_USB

After rebuilding the firmware, run the application.

In a separate terminal, run the usb audio host utility provided in the tools/audio folder:

.. code-block:: console

        $ process_wav.sh -c 4 input.wav output.wav

This application requires the input audio wav file to be 4 channels in the order MIC 0, MIC 1, REF L, REF R.  Output is ASR, ignore, REF L, REF R, MIC 0, MIC 1, where the reference and microphone are passthrough.

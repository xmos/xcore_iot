#########
Audio Mux
#########

This example application can be configured for onboard mic, usb audio, or i2s input.  Outputs are usb audio and i2s.  No DSP is performed on the audio, but the example contains an empty 2 tile pipeline skeleton for a user to populate. In this example all USB audio endpoints are sychronous.

******************
Preparing the host
******************

On Linux and macOS the user may need to update their ``udev`` rules for USB configurations.  Add a custom ``udev`` rule for USB device with VID ``0x20B1`` and PID ``0x0021``.

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the repository root folder.

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_audio_mux

====================
Running the firmware
====================

Run the following commands in the build folder.

.. code-block:: console

    make run_example_audio_mux

================================
Debugging the firmware with xgdb
================================

Run the following commands in the build folder.

.. code-block:: console

    make debug_example_audio_mux

***********************************
Deploying the firmware with Windows
***********************************

=====================
Building the firmware
=====================

Run the following commands in the repository root folder.

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_audio_mux

====================
Running the firmware
====================

Run the following commands in the build folder.

.. code-block:: console

    nmake run_example_audio_mux

================================
Debugging the firmware with xgdb
================================

Run the following commands in the build folder.

.. code-block:: console

    nmake debug_example_audio_mux
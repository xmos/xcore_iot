##############
Device Control
##############

This example application demonstrates how to use device control over USB and I2C.

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_iot root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_freertos_device_control

====================
Running the firmware
====================

From the build folder run:

.. code-block:: console

    make run_example_freertos_device_control

================================
Debugging the firmware with xgdb
================================

From the build folder run:

.. code-block:: console

    make debug_example_freertos_device_control

=============================
Building the host application
=============================

With the firmware running in its own terminal, in a new window,
run the following commands in the xcore_iot root folder to build the host app:

.. code-block:: console

    cmake -B build_host
    cd build_host
    make example_freertos_device_control_host

============================
Running the host application
============================

From the `xcore_iot/build_host/examples/freertos/device_control/host` folder run:

.. code-block:: console

    ./example_freertos_device_control_host -g test_cmd

***********************************
Deploying the firmware with Windows
***********************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_iot root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_freertos_device_control

====================
Running the firmware
====================

From the build folder run:

.. code-block:: console

    nmake run_example_freertos_device_control

================================
Debugging the firmware with xgdb
================================

From the build folder run:

.. code-block:: console

    nmake debug_example_freertos_device_control

=============================
Building the host application
=============================

With the firmware running in its own terminal, in a new window,
run the following commands in the xcore_iot root folder to build the host app:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build_host
    cd build_host
    nmake example_freertos_device_control_host

============================
Running the host application
============================

From the `xcore_iot/build_host/examples/freertos/device_control/host` folder run:

.. code-block:: console

    example_freertos_device_control_host.exe -g test_cmd

****************************
Verifying a successful build
****************************

After running the host application, you should see the following output in your console:

.. code-block:: console

    Command test_cmd sent with resid 3
    Bytes received are:
    50462976


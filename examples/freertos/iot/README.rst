===
IoT
===

This example demonstrates how to control GPIO using MQTT.

.. note::

    This example application is currently only supported on Linux or Mac.

************************
Networking configuration
************************

In this example, we demonstrate using the Eclipse Mosquitto MQTT broker.  Ensure that you have installed Mosquitto by following the instructions
here: https://mosquitto.org/download/.

.. note::

    You can modify the example code to connect to a different MQTT broker.  When doing so, you will also need to modify the filesystem setup scripts before running them.  THis is to ensure that the correct client certificate, private key, and CA certificate are flashed.  See ``filesystem_support/create_fs.sh`` and the instructions for setting up the filesystem below.

Next, configure the example software to connect to the proper MQTT broker.  If you are running the MQTT broker on your local PC, you will need to know that PC's IP address.  This can be determined a number of ways including:

.. code-block:: console

    ifconfig

Lastly, in ``appconf.h``, set ``appconfMQTT_HOSTNAME`` to your MQTT broker IP address or URL:

.. code-block:: c

    #define appconfMQTT_HOSTNAME "your endpoint here"

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_iot

***********************
Setting up the hardware
***********************

.. note::
   The host applications are required to create the filesystem.  See the SDK Installation instructions for more information.

Before the demo can be run, the filesystem must be configured and flashed.

.. tab:: Linux and Mac

    .. code-block:: console

        make flash_fs_example_freertos_iot

The script will create TLS credentials and prompt you for WiFi credentials:

.. code-block:: console

    Enter the WiFi network SSID:
    Enter the WiFi network password:
    Enter the security (0=open, 1=WEP, 2=WPA):
    Add another WiFi network? (y/n):

.. note::

    Once a WiFi profile has been created it will automatically be used.  If you need to change the profile, delete ``networks.dat``.

********************
Running the firmware
********************

Run the following commands in the xcore_sdk root folder:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_freertos_iot

*********************
Testing MQTT Messages
*********************

Running the broker
==================

From the root folder of the iot example run:

.. code-block:: console

    cd mosquitto
    mosquitto -v -c mosquitto.conf

Note: You may need to modify permissions of the cryptocredentials for mosquitto to use them.

Sending messages
================

To turn LED 0 on run:

.. code-block:: console

    mosquitto_pub --cafile mqtt_broker_certs/ca.crt --cert mqtt_broker_certs/client.crt --key mqtt_broker_certs/client.key -d -t "explorer/ledctrl" -m "{"LED": "0",: "status": "on"}"

Supported values for "LED" are ["0", "1", "2", "3"], supported values for "status" are ["on", "off"].

###
IoT
###

This example demonstrates how to control GPIO using MQTT.

************************
Networking configuration
************************

In this example, we demonstrate using the Eclipse Mosquitto MQTT broker.  Ensure that you have installed Mosquitto by following the instructions
here: https://mosquitto.org/download/.

.. note::

    You can modify the example code to connect to a different MQTT broker.  When doing so, you will also need to modify the filesystem setup scripts before running them.  This is to ensure that the correct client certificate, private key, and CA certificate are flashed.  See ``filesystem_support/create_fs.sh`` and the instructions for setting up the filesystem below.

Next, configure the example software to connect to the proper MQTT broker.  If you are running the MQTT broker on your local PC, you will need to know that PC's IP address.  This can be determined a number of ways including the ``ifconfig`` and ``ipconfig`` commands in Linux/macoS and Windows operating systems, respectively.

Lastly, in ``appconf.h``, set ``appconfMQTT_HOSTNAME`` to your MQTT broker IP address or URL:

.. code-block:: c

    #define appconfMQTT_HOSTNAME "your endpoint here"

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the repo's root folder:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_freertos_iot

=======================
Setting up the hardware
=======================

.. note::

   The host applications are required to create the filesystem.  See the installation instructions for more information.

Before the demo can be run, the filesystem must be configured and flashed.

.. code-block:: console

    make flash_app_example_freertos_iot

The script will create TLS credentials and prompt you for WiFi credentials:

.. code-block:: console

    Enter the WiFi network SSID:
    Enter the WiFi network password:
    Enter the security (0=open, 1=WEP, 2=WPA):
    Add another WiFi network? y/[n]:
    Enter the MQTT server's IP/hostname:

.. note::

    The MQTT server's IP/hostname is what is entered into the "Common Name" (CN) of the certification generation process.
    If a hostname was specified for the MQTT server, a DNS server must be available that is configured to resolve that name.

.. note::

    Once these files have been created they will be automatically be used. If the WiFi profile or MQTT certificates/keys
    need to be changed, delete the corresponding components (i.e. ``networks.dat`` or files contained in ``mqtt_broker_certs``).

====================
Running the firmware
====================

Run the following commands in the repo's build folder:

.. code-block:: console

    make run_example_freertos_iot

***********************************
Deploying the firmware with Windows
***********************************

In order to generate the certificates/keys, ``OpenSSL`` must be installed. There are various options for obtaining
a Windows version of ``OpenSSL`` that include ``MinGW`` and ``Git`` installations as well as standalone installations.

Prior to running the commands below, ensure the host system has been setup to permit PowerShell execution.
By default, Windows systems are set to the ``Restricted`` execution policy, for more information see
`about_Execution_Policies <https://go.microsoft.com/fwlink/?LinkID=135170>`__. Setting the policy to ``RemotedSigned``
should be sufficient for proper execution; this can be set from an Administrator PowerShell instance via the command:

.. code-block:: console

    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned

.. note::
    These scripts are not digitally signed, so depending on how they were acquired/downloaded, the policy set above may
    still prevent execution. These files may be unblocked via PowerShell using the cmdlet ``Unblock-File``.

=====================
Building the firmware
=====================

Run the following commands in the repo's root folder:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_freertos_iot

=======================
Setting up the hardware
=======================

.. note::

   The host applications are required to create the filesystem.  See the installation instructions for more information.

Before the demo can be run, the filesystem must be configured and flashed.

.. code-block:: console

    nmake flash_app_example_freertos_iot

The script will create TLS credentials and prompt you for WiFi credentials:

.. code-block:: console

    Enter the WiFi network SSID:
    Enter the WiFi network password:
    Enter the security (0=open, 1=WEP, 2=WPA):
    Add another WiFi network? y/[n]:
    Enter the MQTT server's IP/hostname:

.. note::

    The MQTT server's IP/hostname is what is entered into the "Common Name" (CN) of the certification generation process.
    If a hostname was specified for the MQTT server, a DNS server must be available that is configured to resolve that name.

.. note::

    Once these files have been created they will be automatically be used. If the WiFi profile or MQTT certificates/keys
    need to be changed, delete the corresponding components (i.e. ``networks.dat`` or files contained in ``mqtt_broker_certs``).

====================
Running the firmware
====================

Run the following commands in the repo's build folder:

.. code-block:: console

    nmake run_example_freertos_iot

*********************
Testing MQTT Messages
*********************

Running the broker
==================

From the root folder of the iot example run:

.. code-block:: console

    cd mosquitto
    mosquitto -v -c mosquitto.conf

.. note::

    You may need to modify permissions of the cryptocredentials for mosquitto to use them.

Sending messages
================

To turn LED 0 on, from the IoT example's `filesystem_support` subdirectory, run
the following command (replacing `<MQTT_SERVER>` with the value used during
certificate generation):

.. code-block:: console

    mosquitto_pub -h <MQTT_SERVER> --cafile mqtt_broker_certs/ca.crt --cert mqtt_broker_certs/client.crt --key mqtt_broker_certs/client.key -d -t "explorer/ledctrl" -m '{"LED": "0", "status": "on"}'

Supported values for "LED" are ["0", "1", "2", "3"], supported values for "status" are ["on", "off"].

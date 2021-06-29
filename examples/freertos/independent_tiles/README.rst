#################
Independent Tiles
#################

The `independent_tiles` code example is provides tests of the intertile RPC drivers.  This code example requires the Explorer Board.

****************************
Running the project firmware
****************************

Begin by flashing the WiFi module firmware and setting up the WiFi network:

.. code-block:: console

    $ ./flash_image.sh


To build and run the firmware, run the following commands:

.. code-block:: console

    $ make
    $ make run

Terminate the running firmware with `Ctrl+C`

***************************
Running the intertile tests
***************************

Modify `app_conf.h` to configure which tests to run and which drivers to configure in RPC mode.

Intertile
=========

The intertile test is a stress test for the throughput between tiles. It takes several minutes to run and will print a success message when completed.

GPIO
====

Press button 0, 1 or 0 and 1 to verify the GPIO test.

SwMem
=====

The SwMem test verifies that the SwMem memory segment is accessable from both tiles. It runs quickly and will print a success message when completed.

WiFi
====

The WiFi test verifies that the WiFi driver can be used from either tile. It will print a success message once completed.  Note, you will need to run the `flash_image.sh` script.

RPC
===

The RPC test checks that the RPC host and client is functioning. It will run forever, terminate with `Ctrl+C`.

QSPI Flash
==========

The QSPI Flash test check reads and writes to the flash device. It takes a very long time to run and will print a success message when completed.



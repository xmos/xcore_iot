=================
Independent Tiles
=================

This example application demonstrates how to build a project with a unique FreeRTOS kernel per tile.  The example implements a distributed computing setup, utilizing the intertile device to allow one kernel to perform a remote procedure call on the other tile.

*********************
Building the firmware
*********************

To build this project run:

.. code-block:: console

    $ make BOARD=XCORE-AI-EXPLORER -j

Running the firmware
====================

Running with hardware.

.. code-block:: console

    $ xrun --xscope bin/independent_tiles.xe

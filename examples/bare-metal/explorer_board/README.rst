##############
Explorer Board
##############

This example application demonstrates various capabilities of the Explorer board.

The example consists of pdm_mics to a simple audio processing pipeline which
applies a variable gain.  Pressing button 0 will increase the gain.  Pressing
button 1 will decrease the gain.  The processed audio is sent to the DAC.

When button 0 is pressed, LED 0 will be lit.  When button 1 is pressed, LED 1
will be lit.  When the gain adjusted audio passes a frame power threshold, LED 2
will be lit.  Lastly, LED 3 will blink periodically.

Additionally, the example demonstrates a simple flash and SPI setup.

*********************
Building the firmware
*********************

.. tab:: Linux and Mac

    Run cmake:

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make

    To install, run:

    .. code-block:: console

        $ make install

.. tab:: Windows XTC Tools CMD prompt

    Run cmake:

    .. code-block:: console

        > cmake -G "NMake Makefiles" -B build
        > cd build
        > nmake

    To install, run:

    .. code-block:: console

        $ nmake install

Running the firmware
====================

To run the demo navigate to the bin folder and use the command:

.. code-block:: console

    $ xrun --xscope bin/explorer_board.xe

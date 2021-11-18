##############
Device Control
##############

This example application demonstrates how to use device control over USB and I2C.

*********************************
Building and running the firmware
*********************************

Run the following commands to build the dispatch_queue firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DBOARD=XCORE-AI-EXPLORER
        $ cd build
        $ make

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DBOARD=XCORE-AI-EXPLORER
        $ cd build
        $ nmake
        
To run the example:

.. code-block:: console

    $ xrun --xscope bin/device_control.xe

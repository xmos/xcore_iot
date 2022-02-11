################
L2 Cache Example
################

The L2 cache example demonstrates how to use the software defined L2 cache.

***************************
Building the firmware
***************************

Run the following commands to build the examples:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make l2_cache

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake l2_cache


********************************
Setting up the hardware
********************************

Before running the firmware, the swmem must be flashed.

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make flash_l2_cache_swmem

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake flash_l2_cache_swmem

********************************
Running the firmware
********************************

Running with hardware.

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_l2_cache

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake run_l2_cache

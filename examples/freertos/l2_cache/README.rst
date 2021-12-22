################
L2 Cache Example
################

The L2 cache example demonstrates how to use the software defined L2 cache.

***************************
Building & running examples
***************************

Run the following commands to build the examples:

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

    $ make flash
    $ make run

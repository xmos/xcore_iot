##################
l2_cache Example
##################

The l2_cache example is demonstrates how to use the l2_cache
Note

The function used in this example to multiply two matrices is for illustrative use only. It is not the most efficient way to perform a matrix multiplication. XMOS has optimized libraries specifically for this purpose.

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

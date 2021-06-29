##########
Dispatcher
##########

This example application demonstrates how to create a dispatcher, a dispatch group, and utilize them to parallelize the multiplication of two matrices.  The resulting performance is, as one might expect, four times faster using a dispatcher with four worker threads.

.. note:: The function used in this example to multiply two matrices is for illustrative use only.  It is not the most efficient way to perform a matrix multiplication.  XMOS has optimized libraries specifically for this purpose.

*********************************
Building and running the firmware
*********************************

Run the following commands to build the dispatch_queue firmware:

.. code-block:: console

    $ make
    $ make run

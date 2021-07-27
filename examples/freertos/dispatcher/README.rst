##################
Dispatcher Example
##################

The dispatcher example is demonstrates how to use the dispatcher to parallelize a matrix multiplication operation. Matrix multiplication is a data parallel operation. This means the input matrices can be partitioned and the multiplication operation run on the individual partitions in parallel. A dispatcher is well suited for data parallel problems.

Note

The function used in this example to multiply two matrices is for illustrative use only. It is not the most efficient way to perform a matrix multiplication. XMOS has optimized libraries specifically for this purpose.

***************************
Building & running examples
***************************

Run the following commands to build the examples:

.. code-block:: console

    $ make

To run the example.

.. code-block:: console

    $ make run

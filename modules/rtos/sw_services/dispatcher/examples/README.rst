###################
Dispatcher Examples
###################

The "Hello World" example application demonstrates how to create a dispatcher and add jobs that print out "Hello World". 

The matrix multiplication example is a more advanced and typical example. Matrix multiplication is a data parallel operation. This means the input matrices can be partitioned and the multiplication operation run on the individual partitions in parallel. A dispatcher is well suited for data parallel problems.

Note

The function used in this example to multiply two matrices is for illustrative use only. It is not the most efficient way to perform a matrix multiplication. XMOS has optimized libraries specifically for this purpose.

***************************
Building & running examples
***************************

Run the following commands to build the examples:

.. code-block:: console

    $ cmake -B build
    $ cmake --build build --target install

To run the hello world example.

.. code-block:: console

    $ xrun --xscope --args bin/hello_world.xe

To run the matrix multiplication example.

.. code-block:: console

    $ xrun --xscope --args bin/matrix_multiply.xe

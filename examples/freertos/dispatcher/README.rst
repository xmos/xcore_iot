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

.. tab:: Linux and Mac

	.. code-block:: console

		$ cmake -B build -DBOARD=XCORE-AI-EXPLORER
		$ cd build
		$ make

.. tab:: Windows

	.. code-block:: XTC Tools CMD prompt

		> cmake -G "NMake Makefiles" -B build -DBOARD=XCORE-AI-EXPLORER
		> cd build
		> nmake
		
To run the example:

.. tab:: Linux and Mac

	.. code-block:: console

		$ xrun --xscope bin/dispatcher.xe

.. tab:: Windows

	.. code-block:: XTC Tools CMD prompt

		> xrun --xscope bin\dispatcher.xe
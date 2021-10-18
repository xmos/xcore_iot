##############
Explorer Board
##############

This example application demonstrates various capabilities of the Explorer board.

*********************
Building the firmware
*********************

.. tab:: Linux and Mac

	Run cmake:

	.. code-block:: console

		$ cmake -B build
		$ cd build
		$ make

.. tab:: Windows

	Run cmake:

	.. code-block:: XTC Tools CMD prompt

		> cmake -G "NMake Makefiles" -B build
		> cd build
		> nmake

Running the firmware
====================

To run the demo navigate to the bin folder and use the command:

.. tab:: Linux and Mac

	.. code-block:: console

		$ xrun --xscope bin/explorer_board.xe

.. tab:: Windows

	.. code-block:: XTC Tools CMD prompt

		> xrun --xscope bin\explorer_board.xe

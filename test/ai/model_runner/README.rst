############################
Model Runner Regression Test
############################

The `model_runner` test application is designed to regression test several performance metrics including; code size, tensor arena size, and runtime performance. It compares the latests results against expected results found in `regression.yml`.  Test results are compared to expected results for the appropriate memory segments; SRAM, EXTMEM & SWMEM.

The `run_test.sh` script will generate the model runners, build the applications, run the applications, and generate a test report.

To generate the regression test input report, ensure you have your XCORE-AI-EXPLORER board's XTAG connected and the board is powered on, then run:

.. code-block:: console

    $ ./run_tests.sh

To run the regression tests:

.. code-block:: console

    $ pytest -v

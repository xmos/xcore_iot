# Model Firmware Regression Test

The `xcore_model_firmware` application is designed to regression test several performance metrics including; code size, tensor arena size, and runtime performance. It compares the latests results against known results found in `regression.yml`.  Results are compared for the appropriate memory segments; SRAM, EXTMEM & SWMEM.

The `run_test.sh` script will generate the model runners, build the applications, run the applications, and run the regression tests.

To run the regression, ensure you have your XCORE-AI-EXPLORER board's XTAG connected and the board is powered on, then enter:

    $ ./run_tests.sh


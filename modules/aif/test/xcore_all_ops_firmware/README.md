# All Ops Firmware Regression Test

The `xcore_all_ops_firmware` application is designed to regression test the code size of a minimal application built with all builtin and custom operators.

NOTE: This test is not currently automated!

To run the regression, run the following commands:

    $ cmake -B build
    $ cmake --build build

Manually compare the code size reported by the build to the expected code size in `regression.yml`.

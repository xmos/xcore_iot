######################
RTOS Driver WiFi Tests
######################

This test is designed to perform basic regression tests of the RTOS WIFI driver.

This test assume that the associated RTOS and HILs used have been verified by their own localized separate testing and that the rtos_drivers tests are passing.

This test should be run whenever the code or submodules in `modules\rtos` or `modules\hil` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

*************
Running Tests
*************

Run the tests with the following command:

.. code-block:: console

    $ ./build_and_run_tests.sh

Alternatively, if the firmware is built separately, the tests alone can be run with the following command:

.. code-block:: console

    $ ./run_tests.sh

This test is primarily used by the CI system.  However, it may be useful for developers when modifying the SDK.

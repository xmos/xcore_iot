############################
RTOS Driver Additional Tests
############################

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- spi
- uart

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

To setup the board for testing, refer to the Hardware Setup in `RTOS Driver HIL Tests <https://github.com/xmos/xcore_sdk/blob/develop/test/rtos_drivers/hil/README.rst>`_

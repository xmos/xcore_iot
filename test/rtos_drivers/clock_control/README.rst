#################
RTOS Clock Driver
#################

The RTOS driver tests are designed to regression test RTOS driver behavior for the following drivers:

- clock driver

These tests assume that the associated RTOS and HILs used have been verified by their own localized separate testing.

These tests should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

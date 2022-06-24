#####################
RTOS Driver USB Tests
#####################

This test is designed to perform basic regression tests of the RTOS USB driver.

This test assume that the associated RTOS and HILs used have been verified by their own localized separate testing and that the rtos_drivers tests are passing.

This test should be run whenever the code or submodules in ``modules\rtos`` or ``modules\hil`` are changed.

**************
Hardware Setup
**************

The target hardware for these this is the XCORE-AI-EXPLORER board.

**********
Host Setup
**********

These tests require alsa-utils for the ``aplay`` and ``arecord`` utilities.

Example for Debian:

.. code-block:: console

    $ apt-get install alsa-utils

*************
Running Tests
*************

This test requires two scripts to be run manually.

Run the test with the following command:

.. code-block:: console

    $ ./build_and_run_tests.sh

Alternatively, if the firmware is built separately, the tests alone can be run with the following command:

.. code-block:: console

    $ ./run_tests.sh

While the test is running the user must wait until the audio interface is ready.  On some OS, the interface is opened and closed multiple times.  The firmware is ready for the host script to be run when the console output shows:

.. code-block:: console

    $ ./run_tests.sh
    ...
    Tile[0]|FCore[2]|726621100|USB|uac_loopback_test|Set audio interface 1 alt 1
    Tile[0]|FCore[2]|726673228|USB|uac_loopback_test|Set audio interface 2 alt 1
    Tile[0]|FCore[2]|1726927158|USB|uac_loopback_test|Close audio interface 2 alt 0

Run the host side of the test with the command:

.. code-block:: console

    $ ./run_tests_host.sh

This test is primarily used by the CI system.  However, it may be useful for developers when modifying the SDK.

#########################
Example Application Tests
#########################

The example application tests are designed to regression test behavior for many of the example applications.  These tests should be run whenever the code of an example is changed.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

*************
Running Tests
*************

**NOTE: The example application tests are a work in progress.  Expect changes here soon.**

FreeRTOS Examples
=================

First, build all the FreeRTOS example applications.

.. code-block:: console

    bash tools/ci/build_rtos_core_examples.sh

Run the tests with the following commands:

.. code-block:: console

    bash test/examples/run_freertos_getting_started_tests.sh <optional adapter-id>
    bash test/examples/run_freertos_explorer_board_tests.sh <optional adapter-id>
    bash test/examples/run_freertos_l2_cache_tests.sh <optional adapter-id>
    bash test/examples/run_freertos_tracealyzer_tests.sh <optional adapter-id>


Bare-metal Examples
===================

First, build all the bare-metal example applications.

.. code-block:: console

    bash tools/ci/build_metal_examples.sh

TODO: The bare-metal tests are a work in progress.

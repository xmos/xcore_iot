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

Bare-metal Examples
===================

First, build all the bare-metal example applications.

.. code-block:: console

    $ bash tools/ci/build_metal_examples.sh

Run the tests with the following commands:

.. code-block:: console

    $ bash test/examples/run_bare_metal_vww_tests.sh <optional adapter-id>


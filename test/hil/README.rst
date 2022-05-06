################
HIL Module Tests
################

The HIL module tests are designed to regression test behavior for the following HIL modules:

- i2c
- i2s
- spi
- qspi

These tests should be run whenever the code or submodules in ``modules\hil`` is changed.

**************
Hardware Setup
**************

No hardware is needed for these tests as they all run using the ``xsim`` XCore simulator that is installed with the XTC Tools.

*************
Running Tests
*************

Build and run all the tests with the following command:

.. code-block:: console

    $ ./run_tests.sh

Alternatively, you can build and run tests for a single HIL module (lib_i2c in the following example):

.. code-block:: console

    $ ./run_tests.sh lib_i2c

Alternatively, you can run tests for a single HIL module (lib_i2c in the following example):

.. code-block:: console

    $ pytest lib_i2c

Or, to run just one test (lib_i2c's basic_master in the following example):

.. code-block:: console

    $ pytest lib_i2c/test_basic_master.py::test_i2c_basic_master[400kbps-stop-SCL:1b,SDA:1b]

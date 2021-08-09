############
Contributing
############

*************************************
Contribution Guidelines and Standards
*************************************

Before sending your pull request, make sure your changes are consistent with these guidelines and are consistent with the coding style used in this xcore_sdk repository.

**************************************************
General Guidelines and Philosophy For Contribution
**************************************************

* Include unit tests when you contribute new features, as they help to a) prove that your code works correctly, and b) guard against future breaking changes to lower the maintenance cost.
* Bug fixes also generally require unit tests, because the presence of bugs usually indicates insufficient test coverage.
* Keep API compatibility in mind when you change code.

*******************
Python coding style
*******************

All python code should be `blackened  <https://black.readthedocs.io/en/stable/>`_.
For convenience, the default workspace settings file under `.vscode/` enables format-on-save, and `black` is also provided in the conda environments.

**************************
C, XC and ASM coding style
**************************

Changes to C, C++ or ASM should be consistent with the style of existing C, C++ and ASM code.

`clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__ is used for formatting code.  In most circumstances, the default settings are safe to use.  However, you will need to configure so includes are not sorting.

*******************************
Development Virtual Environment
*******************************

While not required, we recommend you setup an `Anaconda <https://www.anaconda.com/products/individual/>`_ virtual environment.  If necessary, download and follow Anaconda's installation instructions.

Run the following command to create a Conda environment:

.. code-block:: console

    $ conda create --prefix xcore_sdk_venv python=3.8

Run the following command to activate the Conda environment:

.. code-block:: console

    $ conda activate xcore_sdk_venv

Install development dependencies:

.. code-block:: console

    $ ./tools/install/contributing.sh

*****************
Building Examples
*****************

To build the examples, the `XCORE_SDK_PATH` environment variable must be set.

.. code-block:: console

    $ export XCORE_SDK_PATH=<path to>/xcore_sdk

You can also add this export command to your `.profile` or `.bash_profile` script. This way the environment variable will be set in a new terminal window.

Some scripts are provided to build all the example applications.  Run this script with:

.. code-block:: console

    $ bash tools/ci/build_metal_examples.sh all
    $ bash tools/ci/build_rtos_examples.sh all

*************
Running Tests
*************

A script is provided to run all the tests on a connected xcore.ai device.  Run this script with:

.. code-block:: console

    $ bash test/run_tests.sh

****************
Development Tips
****************

At times submodule repositories will need to be updated.  To update all submodules, run the following command

.. code-block:: console

    $ git submodule update --init --recursive

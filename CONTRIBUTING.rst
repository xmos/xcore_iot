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

It is recommended that you install the virtual environment in the repo's directory:

.. code-block:: console

    $ conda env create -p ./xcore_sdk_venv -f tools/develop/environment.yml

Activate the environment by specifying the path:

.. code-block:: console

    $ conda activate xcore_sdk_venv/

Install the SDK:

.. code-block:: console

    $ ./install.sh

To remove the environment, deactivate and run:

.. code-block:: console

    $ conda remove -p xcore_sdk_venv/ --all

*****************
Building Examples
*****************

To build the examples, the `XMOS_AIOT_SDK_PATH` environment variable must be set.

.. code-block:: console

    $ export XMOS_AIOT_SDK_PATH#<path to>/xcore_sdk

You can also add this export command to your `.profile` or `.bash_profile` script. This way the environment variable will be set in a new terminal window.

A script is provided to build all the example applications.  Run this script with:

.. code-block:: console

    $ bash test/build_examples.sh

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

Due to some large submodules, cloning the repository can take a few minutes.  The following command will only close a history depth of 1 and is considerably faster.

.. code-block:: console

    $ git clone --recurse-submodules --depth 1 --shallow-submodules https://github.com/xmos/xcore_sdk.git

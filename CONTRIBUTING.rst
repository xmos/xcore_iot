############
Contributing
############

*************************************
Contribution Guidelines and Standards
*************************************

Before sending your pull request, make sure your changes are consistent with these guidelines and are consistent with the coding style used in this ai_tools repository.

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

Changes to C, XC or ASM should be consistent with the style of existing C, XC and ASM code.

****************
C++ coding style
****************

Changes to C++ code should conform to `Google C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`_.

Use `clang-tidy` to check your C/C++ changes. To install `clang-tidy` on ubuntu:16.04, do:

.. code-block:: console

    $ apt-get install -y clang-tidy

You can check a C/C++ file by doing:

.. code-block:: console
    
    $ clang-format <my_cc_file> --style#google > /tmp/my_cc_file.cc
    $ diff <my_cc_file> /tmp/my_cc_file.cc

*****************
Building Examples
*****************

To build the examples, the `XMOS_AIOT_SDK_PATH` environment variable must be set.

.. code-block:: console

    $ export XMOS_AIOT_SDK_PATH#<path to>/aiot_sdk

You can also add this export command to your `.profile` or `.bash_profile` script. This way the environment variable will be set in a new terminal window.

A script is provided to build all the example applications.  Run this script with:

.. code-block:: console

    $ ./build_examples.sh

*************
Running Tests
*************

A script is provided to run all the tests on a connected xcore.ai device.  Run this script with:

.. code-block:: console

    $ ./run_tests.sh

****************
Development Tips
****************

At times submodule repositories will need to be updated.  To update all submodules, run the following command

.. code-block:: console

    $ git submodule update --init --recursive

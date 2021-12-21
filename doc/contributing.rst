############
Contributing
############

************
Requirements
************

The following software is required for developing code for the SDK.  Install them using your operating systems package management system.

* `Python 3.8 <https://www.python.org/>`__

Install development python packages:

.. code-block:: console

    $ python -m pip install --upgrade pip
    $ pip install -r tools/install/requirements.txt -r tools/install/contribute.txt -r documents/requirements.txt

Python Virtual Environment
==========================

While not required, we recommend you setup an `Anaconda <https://www.anaconda.com/products/individual/>`_ virtual environment.  If necessary, download and follow Anaconda's installation instructions.

Run the following command to create a Conda environment:

.. code-block:: console

    $ conda create --prefix xcore_sdk_venv python=3.8

Run the following command to activate the Conda environment:

.. code-block:: console

    $ conda activate <path-to>/xcore_sdk_venv

Install development packages:

.. code-block:: console

    $ pip install -r tools/install/requirements.txt -r tools/install/contribute.txt -r documents/requirements.txt

Run the following command to deactivate the Conda environment:

.. code-block:: console
    
    $ conda deactivate

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

***************************
C, C++ and ASM coding style
***************************

Changes to C, C++ or ASM should be consistent with the style of existing C, C++ and ASM code.

clang-format
==============

`clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__ is a tool to format source code according to a set of rules and heuristics. 

clang-format can be used to:

- Reformat a block of code to the SDK style. 
- Help you follow the XCore SDK coding style.

The SDK's clang-format configuration file is `.clang-format` and is in the root of the xcore_sdk repository. The rules contained in ``.clang-format`` were originally derived from the Linux Kernel coding style. A few modifications have been made by the XCore SDK authors. Not all code in the XCore SDK follows the ``.clang-format`` rules.  Some non-compliant code is intentional while some is not.  Non-intentional instances should be addressed when the non-compliant code needs to be enhanced.

For more information about `clang-format` visit:

https://clang.llvm.org/docs/ClangFormat.html

https://clang.llvm.org/docs/ClangFormatStyleOptions.html

*******************
Python coding style
*******************

All python code should be `blackened  <https://black.readthedocs.io/en/stable/>`_.

TODO: Add information about the ``black`` config file.

*****************
Building Examples
*****************

Some scripts are provided to build all the example applications.  Run this script with:

.. code-block:: console

    $ bash tools/ci/build_metal_examples.sh all
    $ bash tools/ci/build_rtos_examples.sh all
    $ bash tools/ci/build_rtos_usb_examples.sh all

*************
Running Tests
*************

Tests for most components in the SDK are located in the ``test`` folder.  This includes tests for:

.. toctree::
    :maxdepth: 1
    :includehidden:

    test/hil
    test/rtos_drivers/hil
    test/rtos_drivers/hil_add
    test/rtos_drivers/usb
    test/rtos_drivers/wifi
    test/examples

**************
Adding Modules
**************

If adding a new module to the SDK, the following table may be helpful in determining where the add the module.  If you locate the new module incorrectly, the SDK development team will advice on where and how to relocate it.

.. list-table:: Module Path Descriptions
    :widths: 50 50
    :header-rows: 1

    * - Path
      - Description
    * - modules/modules.cmake
      - General purpose modules
    * - modules/hil/hil.cmake
      - HIL libraries
    * - modules/rtos/rtos.cmake
      - RTOS modules
    * - modules/rtos/drivers/hil.cmake
      - RTOS drivers
    * - modules/rtos/FreeRTOS/kernel.cmake
      - FreeRTOS kernel
    * - modules/rtos/sw_services/sw_services.cmake
      - RTOS software services and middleware

****************
Development Tips
****************

At times submodule repositories will need to be updated.  To update all submodules, run the following command

.. code-block:: console

    $ git submodule update --init --recursive

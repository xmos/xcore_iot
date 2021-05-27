########
AI Tests
########

The AI test requires that you have the development Conda environment setup (see CONTRIBUTING.md) and have installed the AI Tools (see documents/quick_start/installing-ai-tools.rst).

************
Model Runner
************

The `model_runner` tests are designed to regression test several performance metrics including; code size, tensor arena size, and runtime performance. See the README in the `model_runner` folder for additional information.

This test should be run whenever the code or submodules in `modules\aif` are changed.

****************
AI Tools Install
****************

`test_tflite2xcore_install.py` can be used to verify proepr installation of the AI Tools.  Run the test with the following command:

.. code-block:: console

    $ pytest -v test_tflite2xcore_install.py


This test is primarilly used by the CI system.  However, it may be useful for developers to ensure they have the AI Tools properly installed.

*****************
Jupyter Notebooks
*****************

`test_notebooks.sh` verifys Jupyter notebooks in the SDK.  Run the tests with the following command:

.. code-block:: console

    $ ./test_notebooks.sh

This test is primarilly used by the CI system.  However, it may be useful for developers when modifying notebooks in the SDK.

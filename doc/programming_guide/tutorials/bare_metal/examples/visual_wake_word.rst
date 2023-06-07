#################
Visual Wake Words
#################

This example application implements the `Visual Wake Words <https://blog.tensorflow.org/2019/10/visual-wake-words-with-tensorflow-lite_30.html>`__ CNN architecture.  The VWW model is trained to classify images to two classes (person/not-person) and serves as a popular use-case for microcontrollers.

This example demonstrates how to receive input data using ``xscope``.

****************************************
Optimizing the Quantized model for XCORE
****************************************

A quantized model ``vww_quant.tflite`` is included in the model directory.  This model represents a quantized version of the trained VWW model maintained by the `MLPerf™ Tiny Deep Learning Benchmarks for Embedded Devices <https://github.com/mlcommons/tiny>`__ project.  The trained model is located `here <https://github.com/mlcommons/tiny/tree/master/benchmark/training/visual_wake_words/trained_models>`__.

In order to optimize the quantized model, the `XMOS AI Tools <https://pypi.org/project/xmos-ai-tools/>`__ must be installed with the following command:

.. code-block:: console

    pip install xmos-ai-tools

In this example application, the xformer command-line tool from the XMOS AI Tools is used to optimize the model.  The ``xcore-thread-count`` is set to 1, however, you can change the command to utilize additional threads.  In addition, the model in this example is placed in flash so the ``xcore-flash-image-file`` option is used.

.. code-block:: console

    xcore-opt --xcore-thread-count=1 --xcore-flash-image-file=model/vww_model.bin --xcore-conv-err-threshold=0.3421 -o src/vww_model model/vww_quant.tflite

In order to achieve the fasted throughput from the flash device, the model must be nibble swapped.  This can be done with the following command:

.. code-block:: console

    nibble_swap model/vww_model.bin model/vww_model_nibble_swapped.bin

This example demonstrates a portion of the capabilities of the XMOS AI Tools.  Contact your XMOS sales or support representative for more information.

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_iot root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_bare_metal_vww

.. note::

   The host applications are required to create the data partition.  See the XCORE-IOT installation instructions for more information.

From the build folder, create the data partition and flash the device with the following command:

.. code-block:: console

    make flash_app_example_bare_metal_vww

====================
Running the firmware
====================

From the build folder run:

.. code-block:: console

    make run_example_bare_metal_vww

The firmware will now wait until a data is sent from a host application. Test images can be sent to the firmware using ``xscope``.  Most RGB images should work.  Send an image with the ``test_image.py`` Python script using the following command:

.. code-block:: console

    $ ./test_image.py path/to/image

***********************************
Deploying the firmware with Windows
***********************************

=====================
Building the firmware
=====================

Run the following commands in the xcore_iot root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_bare_metal_vww

.. note::

   The host applications are required to create the data partition.  See the XCORE-IOT installation instructions for more information.

From the build folder, create the data partition and flash the device with the following command:

.. code-block:: console

    nmake example_bare_metal_vww

====================
Running the firmware
====================

From the build folder run:

.. code-block:: console

    nmake run_example_bare_metal_vww

The firmware will now wait until a data is sent from a host application. Test images can be sent to the firmware using ``xscope``.  Most RGB images should work.  Send an image with the ``test_image.py`` Python script using the following command:

.. code-block:: console

    $ ./test_image.py path/to/image


#################
Visual Wake Words
#################

This example application implements the `Visual Wake Words <https://blog.tensorflow.org/2019/10/visual-wake-words-with-tensorflow-lite_30.html>`__ CNN architecture.  The VWW model is trained to classify images to two classes (person/not-person) and serves as a popular use-case for microcontrollers.

This example demonstrates how to receive input data using `xscope`.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_bare_metal_vww

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_bare_metal_vww

********************
Running the firmware
********************

Running with hardware

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_example_bare_metal_vww

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake run_example_bare_metal_vww

Running with simulator

.. code-block:: console

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make xsim_example_bare_metal_vww

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake xsim_example_bare_metal_vww

The firmware will now wait until a data is sent from a host application. Test images can be sent to the firmware using `xscope`.  Most RGB images should work.  The `test_image.py` script requires Python.  Ensure you have installed Python 3 and the XCore SDK Python requirements.

Sending a test image to the xcore.ai Explorer board using `xscope`. The `test_image.py` script can be found in the application directory:

.. code-block:: console

    $ ./test_image.py path/to/image

********************
Optimizing the model
********************

An unoptimized, quantized model is included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xcore-opt --xcore-thread-count 5 -o model/model_xcore.tflite model/model_quant.tflite 

Converting flatbuffer to source file
====================================

The following unix command will generate a C source file that contains the TensorFlow Lite model as a char array.

.. code-block:: console

    $ python <path-to-sdk>/tools/tflite_micro/convert_tflite_to_c_source.py --input model/model_xcore.tflite --header src/vww_model_data.h --source src/vww_model_data.c --variable-name vww
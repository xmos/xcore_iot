XMOS AI Toolchain Extenstions User Guide
========================================

Introduction
------------

XCORE is a family of cross-over processors for the IoT and AIoT, helping you to get to the market fast, with products that stand out from the competition.  With XCORE, multiple cores are available for the execution of real-time machine learnig inferencing, decisioning at the edge, signal processing, control and communication - all wrapped up in a single chip.

At the heart of the machine learning inference capabilities lies the vector processing unit (VPU). To keep the system simple and the die area small, while retaining the hard-real-time guarantees of XCORE processors, we decided to integrate the VPU closely with the rest of the ALU. This allows for reduced latency and increased throughput since there is no need to copy memory to/from the accelerator’s memory. Moreover, the VPU breaks with some traditions of traditional RISC architectures.

AI Toolchain Extensions
^^^^^^^^^^^^^^^^^^^^^^^

We have developed a Python module that extends the xTIMEcomposer toolchain and allows model designers and embedded software developers to convert and deploy models to the xcore.ai processor.

The figure below illustrates our neural network deployment workflow, starting with a trained model (with floating point weights and activations) on the left. Our workflow currently targets only `TensorFlow <https://www.tensorflow.org/>`_ models, so you need to convert your model first if you are training in a different framework. Once your model is in TensorFlow, the first step is to convert and quantize it using the built-in `TensorFlow Lite converter <https://www.tensorflow.org/lite/microcontrollers/build_convert>`_. You can do this manually, but we provide you with a helper function that takes a tf.keras Model object and a representative dataset for quantization, then configures and calls the converter/quantizer with the appropriate settings for xcore.ai.

.. figure:: images/model_deployment_workflow.png
    :width: 1024px
    :align: center
    :height: 445px
    :figclass: align-center

    Neural network deployment workflow

Once your model is quantized, the next step is to use our converter to optimize the computational graph for our platform. The xcore.ai optimizer/converter consumes the model in the TensorFlow Lite `flatbuffer <https://google.github.io/flatbuffers/>`_ format output by the TensorFlow Lite converter and outputs another flatbuffer following the same TensorFlow Lite schema. The optimizer’s internal structure is similar to that of a compiler: 

1. The model is converted to our own intermediate representation (IR) of the computational graph
2. Transformations are applied in multiple stages, each stage consisting of multiple passes that mutate the IR, including canonicalization, optimization, lowering, cleanup, and other passes.
3. The IR is serialized (i.e. converted) back to the TensorFlow Lite flatbuffer format.

The most important xcore.ai specific transformations performed are related to the optimization of convolutions and fully connected layers. Due to the VPU's hardware implementation, the most efficient kernel implementation may require the weight tensor to be laid out differently from OHWI (i.e. output channel, kernel height, kernel width, input channel) layout used by TensorFlow Lite. Moreover, some parameters related to quantization can be precalculated and stored interleaved with the biases, reducing overhead and speeding up execution. Beyond the transformation passes that calculate these custom weight, bias and quantization parameter tensors, our converter also plans the parallel execution of the kernels, fuses zero-padding operators with convolutions, and performs many other optimizations.

Below is a list of `TensorFlow Lite for Microcontrollers <https://www.tensorflow.org/lite/microcontrollers>`_ operators that have been optimized for xcore.ai.  Depending on the parameters of the operator, the optimized implementation will be 10-50 times faster than the reference implementation.

- CONV_2D
- DEPTHWISE_CONV_2D
- FULLY_CONNECTED
- MAX_POOL_2D
- AVERAGE_POOL_2D
- MEAN
- LOGISTIC
- RELU
- RELU6

.. note:: Additional operators will be optimized in future releases.

.. note:: TensorFlow Lite for Microcontrollers currently supports a limited subset of TensorFlow operations, which impacts the model architectures that it is possible to run. The TensorFlow community is actively working on expanding operation support.  The supported operations can be seen in the file `all_ops_resolver.cc <https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/all_ops_resolver.cc>`_

It is worth noting that our optimizer is a standalone executable with the input and output model in the same format. This means that you can use it even if you have builtin or custom operators in your model. Our optimizer was designed to not alter unknown custom operators by default but be extensible if implementing optimizations for such operators is desired.

The final step in our optimization workflow is to deploy your model in our port of the TensorFlow Lite for Microcontrollers runtime. The TensorFlow Lite for Microcontrollers project provides the tools necessary to encapsulate the model and the runtime in source files that can be linked with the rest of your embedded project, compiled using our tools, and deployed on the hardware or tested in our cycle-accurate simulator. To execute the optimized graph, our runtime relies on a library of fine-tuned neural network kernels that take full advantage of the xcore.ai VPU. This library is also self-contained, so you can implement the model execution yourself if the computation constraints do not allow the overhead associated with the runtime. This overhead can be anything between 20KB and 160KB, depending on how many and which kernels your model uses, but decreases continuously as we improve our implementation of the runtime and the kernels.

Installation
------------

System Requirements
^^^^^^^^^^^^^^^^^^^

The AI Toolchain Extensions are officially supported on the following platforms:

- MacOS 10.13 +
- Linux CentOS 5.8 or Ubuntu 12.04 LTS

The tools also work on many other versions of Linux, including Fedora 30 +.

*Windows 10 is not currently supported.  However, support for Windows is expected for initial product release*

Installation Step by Step
^^^^^^^^^^^^^^^^^^^^^^^^^

To install the AI toolchain extensions, follow these steps:

**Step 1. Install prerequisites.**

`Python 3.6 <https://www.python.org/downloads/>`_ + is required, however, we recommend you setup an `Anaconda <https://www.anaconda.com/products/individual/>`_ environment before installing.  If necessary, download and follow Anaconda's installation instructions.

`Version 15 of the XMOS Toolchain <https://www.xmos.com/software/tools/>`_ and `CMake 3.14 <https://cmake.org/download/>`_ + are required for building the example applications.  If necessary, download and follow the installation instructions for those components.

**Step 2. Download the XMOS AIoT SDK**

Download and unzip the archive.

**Step 3. Set up the environment variables**

.. code-block:: console

    $ export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

You can also add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set in a new terminal window.

**Step 4. Create a Conda environment**

.. code-block:: console

    $ conda create --prefix xmos_env python=3.6

Activate the environment

.. code-block:: console

    $ conda activate xcore_env

.. note:: You may need to specify the fully-qualified path to your environment.

**Step 5. Install AI Extensions Python module**

.. code-block:: console

    $ pip install -e ${XMOS_AIOT_SDK_PATH}/ai_tools/tflite2xcore

Optimizing and Deploying Your Model
-----------------------------------

We've provided two paths for you to optimize your model for xcore.ai.

The **recommended** option is to use Python or a Jupyter Notebook.  We've provided an `example Notebook <training_and_converting.ipynb>`_ where we demonstrate how to load a TensorFlow CNN model trained using the `CIFAR-10 <https://www.cs.toronto.edu/~kriz/cifar.html>`_ dataset, convert it to TensorFlow Lite, then optimize it to be deployed to xcore.ai.  This notebook requires the installation of some additional Python packages

.. code-block:: console

    $ pip install jupyter
    $ pip install matplotlib

Command-line Python scripts are also provided for those that prefer not to write your own Python scripts or use Jupyter Notebooks.

Model Optimization
^^^^^^^^^^^^^^^^^^

The ``xformer.py`` script is used to transform a quantized TensorFlow Lite model to a format optimized for xcore.ai. Run the following command to transform your model into an optimized version:

.. code-block:: console

    $ xformer.py <input .tflite file> <output .tflite file>

``xformer.py`` has several helpful command-line arguments. 

.. csv-table::
    :header: "Argument", "Help"
    :widths: 10, 50

    "--analyze", "Analyze the output model. A report is printed showing the runtime memory footprint of the model. (default: False)"
    "--num_threads NUM_THREADS", "Number of parallel threads for xcore.ai optimization. (default: 1)"
    "-v, --verbose", "Set verbosity level. -v: summary info of mutations; -vv: detailed mutation and debug info. (default: 0)"

Run the following command to see the full list of command-line arguments:

.. code-block:: console

    $ xformer.py --help

.. _Model_Visualization:

Model Visualization
^^^^^^^^^^^^^^^^^^^

For visualizing the model graph, we recommend using `Netron <https://lutzroeder.github.io/netron/>`_.

Moreover, included in the installation, the ``tflite_visualize.py`` script can also be used to visualize any TensorFlow Lite model, including those optimized for xcore.ai. You can visualize the model conversion with the following command:

.. code-block:: console

    $ tflite_visualize.py <input .tflite file>} -o <output .html file>

Open ``<output .html file>`` in your browser to inspect the model.


Model Deployment
^^^^^^^^^^^^^^^^

Preparing your optimized model for inference on the xcore.ai device involves generating a few source code files.

.. note:: Future releases will contain scripts to automate the model deployment process.

The first step is to convert your model to source code.  We recommend you use the ``convert_file_to_c_source.py`` utility provided by the TensorFlow community.  This script is located in the AIoT SDK under ``ai_tools/third_party/tensorflow/tensorflow/lite/python``.  An example command would look like:

.. code-block:: console

    $ python $XMOS_AIOT_SDK_PATH/third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file <model_xcore.tflite> --output_header_file <model.h> --output_source_file <model.c> --array_variable_name <model> --include_guard <MODEL_H_>

Of course, you will need to replace the details inside the brackets with values you prefer to use in your application firmware.  See the README files of the example firmware applications for instructions on how those models are converted to source code.

Additionally, you need to write code to setup the TensorFlow Lite for Microcontrollers runtime and operator loading.  This is very similar to the code snippets given in the TensorFlow Lite for Microcontrollers `Getting Started Guide <https://www.tensorflow.org/lite/microcontrollers/get_started>`_.  You will want to customize the declaration and setup of the ``tflite::MicroMutableOpResolver`` by registering the necessary operators.  The following code snippet demonstrates:

.. code-block:: cpp

    static tflite::MicroMutableOpResolver<7> resolver;
    resolver.AddSoftmax();
    resolver.AddPad();
    resolver.AddCustom("XC_maxpool2d",
                        tflite::ops::micro::xcore::Register_MaxPool2D());
    resolver.AddCustom("XC_fc_deepin_anyout",
                        tflite::ops::micro::xcore::Register_FullyConnected_16());
    resolver.AddCustom("XC_conv2d_shallowin",
                        tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    resolver.AddCustom("XC_conv2d_deep",
                        tflite::ops::micro::xcore::Register_Conv2D_Deep());
    resolver.AddCustom("XC_requantize_16_to_8",
                        tflite::ops::micro::xcore::Register_Requantize_16_to_8());

You can add as many supported operators as you would like to the ``tflite::MicroMutableOpResolver`` with the ``Add*`` or ``AddCustom`` methods.  However, adding unused operators adds code to the compiled firmware.  We recommend you add only the operators used in your model.  You can determine the used operators by looking at the **Operator Codes** section of the output from ``tflite_visualize.py``.  See Model_Visualization_ for instructions on how to generate the model visualization.

The supported ``Add*`` methods for builtin operators can be seen in the file `all_ops_resolver.cc <https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/all_ops_resolver.cc>`_

The code snippet below demostrates examples for calls to ``AddCustom`` for the xcore.ai custom operators.  The ``Add*`` methods can be called in any order.  The operators do not need to be added in the order they appear in the model. 

.. code-block:: cpp

    resolver.AddCustom("XC_conv2d_shallowin",
                        tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    resolver.AddCustom("XC_conv2d_deep",
                        tflite::ops::micro::xcore::Register_Conv2D_Deep());
    resolver.AddCustom("XC_conv2d_1x1",
                        tflite::ops::micro::xcore::Register_Conv2D_1x1());
    resolver.AddCustom("XC_conv2d_depthwise",
                        tflite::ops::micro::xcore::Register_Conv2D_Depthwise());
    resolver.AddCustom("XC_fc_deepin_anyout",
                        tflite::ops::micro::xcore::Register_FullyConnected_16());
    resolver.AddCustom("XC_maxpool2d",
                        tflite::ops::micro::xcore::Register_MaxPool2D());
    resolver.AddCustom("XC_avgpool2d",
                        tflite::ops::micro::xcore::Register_AvgPool2D());
    resolver.AddCustom("XC_avgpool2d_global",
                        tflite::ops::micro::xcore::Register_AvgPool2D_Global());
    resolver.AddCustom("XC_requantize_16_to_8",
                        tflite::ops::micro::xcore::Register_Requantize_16_to_8());
    resolver.AddCustom("XC_lookup_8",
                        tflite::ops::micro::xcore::Register_Lookup_8());

The ``examples/bare-metal/cifar10`` example is a great place to look at how to deploy a model.  Of course, your application code will vary, but your code for integrating the TensorFlow Lite Micro runtime will be very similar the code in this example located in the ``examples/bare-metal/cifar10/inference_engine/src/`` folder.

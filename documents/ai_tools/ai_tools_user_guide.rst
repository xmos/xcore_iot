.. _ai_tools_user_guide:

###################
AI Tools User Guide
###################

************
Introduction
************

XCore is a family of cross-over processors for the IoT and AIoT, helping you to get to the market fast, with products that stand out from the competition.  With XCore, multiple cores are available for the execution of real-time machine learnig inferencing, decisioning at the edge, signal processing, control and communication - all wrapped up in a single chip.

At the heart of the machine learning inference capabilities lies the vector processing unit (VPU). To keep the system simple and the die area small, while retaining the hard-real-time guarantees of XCore processors, we decided to integrate the VPU closely with the rest of the ALU. This allows for reduced latency and increased throughput since there is no need to copy memory to/from the accelerator’s memory. Moreover, the VPU breaks with some traditions of traditional RISC architectures.

Overview
========

We have developed a Python module that extends the XTC Tools and allows model designers and embedded software developers to optimize models for the xcore.ai processor.

The figure below illustrates our neural network deployment workflow, starting with a trained model (with floating point weights and activations) on the left. Our workflow currently targets only `TensorFlow <https://www.tensorflow.org/>`_ models, so you need to convert your model first if you are training in a different framework. Once your model is in TensorFlow, the first step is to convert and quantize it using the built-in `TensorFlow Lite converter <https://www.tensorflow.org/lite/microcontrollers/build_convert>`_. You can do this manually, but we provide you with a helper function that takes a tf.keras Model object and a representative dataset for quantization, then configures and calls the converter/quantizer with the appropriate settings for xcore.ai.

.. figure:: model_deployment_workflow.png
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

- ADD
- AVERAGE_POOL_2D
- CONV_2D
- DEPTHWISE_CONV_2D
- FULLY_CONNECTED
- LOGISTIC
- MAX_POOL_2D
- MEAN
- RELU
- RELU6

.. note:: Additional operators will be optimized in future releases.

.. note:: TensorFlow Lite for Microcontrollers currently supports a limited subset of TensorFlow operations, which impacts the model architectures that it is possible to run. The TensorFlow community is actively working on expanding operation support.  The supported operations can be seen in the file `all_ops_resolver.cc <https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/all_ops_resolver.cc>`_

It is worth noting that our optimizer is a standalone executable with the input and output model in the same format. This means that you can use it even if you have builtin or custom operators in your model. Our optimizer was designed to not alter unknown custom operators by default but be extensible if implementing optimizations for such operators is desired.

*********************
Optimizing Your Model
*********************

Be sure you have installed the XMOS AI Tools.  If you setup a virtual environment as suggested in the :ref:`Getting Started <ai_tools-setup-virtual-environment-label>` guide, remember to :ref:`activate <ai_tools-activate-virtual-environment-label>` it before proceeding.

We've provided two paths for you to optimize your model for xcore.ai.

The **recommended** option is to use Python or a Jupyter Notebook.  We've provided an `example Notebook <https://github.com/xmos/xcore_sdk/tree/develop/examples/bare-metal/cifar10/train/training_and_converting.ipynb>`_ where we demonstrate how to load a TensorFlow CNN model trained using the `CIFAR-10 <https://www.cs.toronto.edu/~kriz/cifar.html>`_ dataset, convert it to TensorFlow Lite, then optimize it to be deployed to xcore.ai.  This notebook requires the installation of some additional Python packages

.. code-block:: console

    $ pip install jupyter

Command-line Python scripts are also provided for those that prefer not to write your own Python scripts or use Jupyter Notebooks.

Model Optimization
==================

The ``xformer.py`` script is used to transform a quantized TensorFlow Lite model to a format optimized for xcore.ai.

Run the following command to transform your model into an optimized version:

.. code-block:: console

    $ xformer.py <input .tflite file> <output .tflite file>

``xformer.py`` has several helpful command-line arguments. Read more on the :ref:`xformer.py manpage <xformer-manpage>`.

Model Visualization
===================

For visualizing the model graph, we recommend using `Netron <https://lutzroeder.github.io/netron/>`_.

Moreover, included in the installation, the ``tflite_visualize.py`` script can also be used to visualize any TensorFlow Lite model, including those optimized for xcore.ai. You can visualize the model conversion with the following command:

.. code-block:: console

    $ tflite_visualize.py <input .tflite file> -o <output .html file>

Open ``<output .html file>`` in your browser to inspect the model.

Model Deployment
================

The :ref:`AI Deployment User Guide <ai_deployment_user_guide>` provides information on deploying your optimized model to XCORE.

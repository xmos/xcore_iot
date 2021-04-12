.. _ai_framework_user_guide:

#######################
AI Framework User Guide
#######################

************
Introduction
************

After you have :ref:`optimized your model for XCORE <ai_toolchain_extensions_user_guide>`, the final step in our optimization workflow is to deploy your model in our port of the TensorFlow Lite for Microcontrollers runtime. The TensorFlow Lite for Microcontrollers project provides the tools necessary to encapsulate the model and the runtime in source files that can be linked with the rest of your embedded project, compiled using our tools, and deployed on the hardware or tested in our cycle-accurate simulator. To execute the optimized graph, our runtime relies on a library of fine-tuned neural network kernels that take full advantage of the xcore.ai VPU. This library is also self-contained, so you can implement the model execution yourself if the computation constraints do not allow the overhead associated with the runtime. This overhead can be anything between 20KB and 160KB, depending on how many and which kernels your model uses, but decreases continuously as we improve our implementation of the runtime and the kernels.

Recommended Model Deployment
============================

Preparing your optimized model for inference on the xcore.ai device involves generating a model runner.

To generate a model runner, we recommend you use the ``generate_model_runner.py`` utility provided.  An example command would look like:

.. code-block:: console

    $ generate_model_runner.py <model_xcore.tflite> --name <runner_name>

Of course, you will need to replace the details inside the brackets with values you prefer to use in your application firmware.  

This command will generate four source code files, including a C API that you can integrate into your application firmware to run inference using the model.  Also generated is a C source file that contains the TensorFlow Lite model as a character array.  This model can be stored in SRAM, LPDDR, or extracted to be placed in flash.  See :ref:`generate_model_runner.py manpage <generate_model_runner-manpage>` for additional information.

The code block before demonstrates the steps needed to integrate the model runner into your applications.

.. code-block:: c

    // Declare the model runner tensor arena memory buffer for 
    // scratch and activation buffer storage.  In this example
    // code we have alloted 100kB for the tensor arena.  Your
    // models requirements will be different.
    // NOTE: The tensor arena can NOT be placed in the ExtMem memory segment.
    #define TENSOR_ARENA_SIZE 100000
    static unsigned char tensor_arena[TENSOR_ARENA_SIZE];

    // Declare the model runner context
    static model_runner_t model_runner_ctx_s;
    static model_runner_t *model_runner_ctx = &model_runner_ctx_s;

    // Declare some useful variables
    static int8_t *input_buffer;
    static size_t input_size;
    static int8_t *output_buffer;

    // Initialize & create a model runner context
    model_runner_init(tensor_arena, TENSOR_ARENA_SIZE);
    model_runner_create(model_runner_ctx, cifar10_model_data);

    // Set some useful variables
    input_buffer = model_runner_get_input(model_runner_ctx);
    input_size = model_runner_get_input_size(model_runner_ctx);
    output_buffer = model_runner_get_output(model_runner_ctx);

    // Set the input_buffer with your input values
    // ...

    // Run inference
    model_runner_invoke(model_runner_ctx);
    
    // Do something with the output_buffer
    // ...

The ``examples/bare-metal/cifar10`` example is a great place to look at how to generate a model runner.  Of course, your application code will vary, but your code for integrating the TensorFlow Lite Micro runtime will be very similar the code in this example located in the ``examples/bare-metal/cifar10/model_runner/src/`` folder.

Converting flatbuffer to source file
------------------------------------

If you choose not to use ``generate_model_runner.py`` or you have already utilized ``generate_model_runner.py`` but the model has been modified slightly, you will need to convert your model to source code.  We recommend you use the ``convert_tflite_to_c_source.py`` utility provided.  An example command would look like:

.. code-block:: console

    $ python convert_tflite_to_c_source.py --input <model_xcore.tflite> --header <model_data.h> --source <model_data.c> --variable-name <model> --include-guard <MODEL_H_>

You will need to replace the details inside the brackets with values you prefer to use in your application firmware.  See the README files of the example firmware applications for instructions on how those models are converted to source code. See :ref:`convert_tflite_to_c_source.py manpage <convert_tflite_to_c_source-manpage>` for additional information.

Manual Model Deployment
=======================

While not recommended, it is possible to deploy your model manually.  Understanding the manual model deployment process will also help you understand the code generated by the ``generate_model_runner.py`` utility described above.  To begin with, you need to write C++ code to setup the TensorFlow Lite for Microcontrollers runtime and operator registration.  This is very similar to the code snippets given in the TensorFlow Lite for Microcontrollers `Getting Started Guide <https://www.tensorflow.org/lite/microcontrollers/get_started>`_ .  You will want to customize the declaration and setup of the ``tflite::MicroMutableOpResolver`` by registering the necessary operators.  The following code snippet demonstrates:

.. code-block:: cpp

    // NOTE: Don't forget to increment the template argument if you
    //       add another operator.
    static tflite::MicroMutableOpResolver<7> resolver;
    resolver.AddPad();
    resolver.AddSoftmax();
    resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Deep_OpCode,
                       tflite::ops::micro::xcore::Register_Conv2D_Deep());
    resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode,
                       tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    resolver.AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode,
                       tflite::ops::micro::xcore::Register_FullyConnected_8());
    resolver.AddCustom(tflite::ops::micro::xcore::MaxPool2D_OpCode,
                       tflite::ops::micro::xcore::Register_MaxPool2D());

You can add up to 128 operators to the ``tflite::MicroMutableOpResolver`` with the ``Add*`` or ``AddCustom`` methods.  However, adding unused operators adds code to the compiled firmware.  We recommend you add only the operators used in your model.  You can use the `Netron <https://lutzroeder.github.io/netron/>`_ visualization tool determine operators required for your model.

The supported ``Add*`` methods for builtin operators can be seen in the file `all_ops_resolver.cc <https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/all_ops_resolver.cc>`_

The code snippet below demostrates examples for calls to ``AddCustom`` for the xcore.ai custom operators.  The ``Add*`` methods can be called in any order.  The operators do not need to be added in the order they appear in the model.  And, an operator only needs to be added once, even if it appears multiple times in your model.

.. code-block:: cpp

    resolver.AddCustom(tflite::ops::micro::xcore::Add_8_OpCode,
                       tflite::ops::micro::xcore::Register_Add_8());
    resolver.AddCustom(tflite::ops::micro::xcore::AvgPool2D_OpCode,
                       tflite::ops::micro::xcore::Register_AvgPool2D());
    resolver.AddCustom(tflite::ops::micro::xcore::AvgPool2D_Global_OpCode,
                       tflite::ops::micro::xcore::Register_AvgPool2D_Global());
    resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_1x1_OpCode,
                       tflite::ops::micro::xcore::Register_Conv2D_1x1());
    resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Deep_OpCode,
                       tflite::ops::micro::xcore::Register_Conv2D_Deep());
    resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Depthwise_OpCode,
                       tflite::ops::micro::xcore::Register_Conv2D_Depthwise());
    resolver.AddCustom(tflite::ops::micro::xcore::Conv2D_Shallow_OpCode,
                       tflite::ops::micro::xcore::Register_Conv2D_Shallow());
    resolver.AddCustom(tflite::ops::micro::xcore::FullyConnected_8_OpCode,
                       tflite::ops::micro::xcore::Register_FullyConnected_8());
    resolver.AddCustom(tflite::ops::micro::xcore::Lookup_8_OpCode,
                       tflite::ops::micro::xcore::Register_Lookup_8());
    resolver.AddCustom(tflite::ops::micro::xcore::MaxPool2D_OpCode,
                       tflite::ops::micro::xcore::Register_MaxPool2D());

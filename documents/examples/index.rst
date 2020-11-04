###############################
Bare-Metal Example Applications
###############################

The examples are separated into bare-metal and FreeRTOS based applications. The following sections describe each of the examples and point to the location of the specific build and usage of each.

***********
Hello World
***********

Demonstrates the absolute basics of using TensorFlow Lite for Microcontrollers, by using a trained model to estimate sin(x), and represent a sine wave amplitude output on Explorer Board LEDs.

.. code-block:: console

    examples\bare-metal\hello_world


`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/bare-metal/hello_world/README.md>`__

********
CIFAR-10
********

This example application implements a CNN architecture trained on the CIFAR-10 dataset. This example demonstrates how to place models in SRAM, Flash or LPDDR.  And, how to benchmark your inference.

.. code-block:: console

    examples\bare-metal\cifar10

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/bare-metal/cifar10/README.md>`__

************
Micro Speech
************

This example application is the `micro_speech <https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech>`_ example from TensorFlow Lite for Microcontrollers.

.. code-block:: console

    examples\bare-metal\micro_speech

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/bare-metal/micro_speech/README.md>`__


************
MobileNet V1
************

This example application implements a `MobileNet V1 <https://arxiv.org/abs/1704.04861>`_ CNN architecture trained on the `ImageNet <http://www.image-net.org/>`_ dataset.  This example demonstrates how to place models in LPDDR and to recieve input data using `xscope`.

.. code-block:: console

    examples\bare-metal\mobilenet_v1

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/bare-metal/mobilenet_v1/README.md>`__

#############################
FreeRTOS Example Applications
#############################

********
CIFAR-10
********

This example application implements a CNN architecture trained on the CIFAR-10 dataset. This example demonstrates how to place models in SRAM, Flash or LPDDR.  And, the example reads a set of test images from a filesystem stored in flash. 

.. code-block:: console

    examples\freertos\cifar10

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/freertos/cifar10/README.md>`__


**************
Explorer Board
**************

This example application shows how to use the lib_soc and FreeRTOS libraries to make FreeRTOS applications targetting xCORE. The application uses I2C, I2S, Ethernet, mic array, and GPIO devices.

.. code-block:: console

    rtos_support\examples\app_freertos_explorer_board

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/freertos/explorer_board/README.md>`__

*****************
Independent Tiles
*****************

This example application illustrates cross tile communcation from FreeRTOS.

.. code-block:: console

    rtos_support\examples\app_freertos_independent_tiles

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/freertos/app_freertos_independent_tiles/README.md>`__

*******
AWS IoT
*******

The XMOS XCOREAI Explorer Board provides connectivity to Amazon IoT Core. Combined with Alexa Voice Service (AVS) and AWS Lambda, this demo allows you to quickly control GPIO using a custom Alexa Skill. This example provides instructions on how to configure the development board, create your Alexa skill, and tie an Alexa skill intent to AWS IoT Core using a Lambda function.

.. code-block:: console

    rtos_support\examples\app_iot_aws

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/freertos/iot_aws/README.md>`__

****************
Person Detection
****************

This example demonstrates how to integrate the xCORE FreeRTOS port with the TensorFlow person detection example application.  This application places the model in LPDDR.

.. code-block:: console

    examples\freertos\person_detection

`Documentation <https://github.com/xmos/aiot_sdk/blob/develop/examples/freertos/person_detection/README.md>`__

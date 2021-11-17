.. _generate_model_runner-manpage:

.. program:: generate_model_runner.py

########################
generate_model_runner.py
########################

********
Synopsis
********

.. code-block::

    generate_model_runner.py [-h] --input INPUT [--output OUTPUT]
                             [--analyze] [--name NAME]

***********
Description
***********

The ``generate_model_runner.py`` script is used to generate a model runner project for given .tflite model file(s).

Usage
=====

.. code-block:: console

    $ generate_model_runner.py --input <tflite_input> --output <output_path> --name <runner_name>

*******
Options
*******


Overall Options
===============

.. option:: --input

    Full filepath of the input TensorFlow Lite file.
    Multiple entries are supported.

.. option:: --output

    Full filepath of the output directory where source files will be generated.

.. option:: --name <NAME>

    Name to use for the model runner.

.. option:: --analyze

    Analyze the output model. A report is printed showing the
    runtime memory footprint of the model.

.. option:: -h, --help

    Print help message. 

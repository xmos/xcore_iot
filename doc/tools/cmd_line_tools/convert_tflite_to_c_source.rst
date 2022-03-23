.. _convert_tflite_to_c_source-manpage:

.. program:: convert_tflite_to_c_source.py

#############################
convert_tflite_to_c_source.py
#############################

********
Synopsis
********

.. code-block::

    convert_tflite_to_c_source.py [-h] --input INPUT
                                  [--variable-name VARIABLE_NAME]
                                  [--source SOURCE] [--header HEADER]
                                  [--include-guard INCLUDE_GUARD]
                                  [--line-width LINE_WIDTH]

***********
Description
***********

The ``convert_tflite_to_c_source.py`` script is used to generate source and header files from a TensorFlow Lite model.

Usage
=====

.. code-block:: console

    $ convert_tflite_to_c_source.py --input <tflite_input> --source <source_output> --header <header_output>

*******
Options
*******

Overall Options
===============

.. option:: --input

    Full filepath of the input TensorFlow Lite file.

.. option:: --source

    Full filepath of the output C source file.

.. option:: --header

    Full filepath of the output C header file.

.. option:: --variable-name <VARIABLE_NAME>

     Name to use for the C data array variable.

.. option:: --include-guard <INCLUDE_GUARD>

     Name to use for the C header include guard.

.. option:: --line-width <LINE_WIDTH>

     Width to use for formatting.

.. option:: -h, --help

    Print help message. 

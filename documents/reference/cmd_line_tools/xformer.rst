 .. _xformer-manpage-label:

.. program:: xformer.py

##########
xformer.py
##########

********
Synopsis
********

.. code-block::

    xformer.py    [-h] [-v] [--minify] [-par NUM_THREADS]
                  [--intermediates_path INTERMEDIATES_PATH] [--analyze] [--version]
                  tflite_input tflite_output

***********
Description
***********

The ``xformer.py`` script is used to transform a quantized TensorFlow Lite model to a format optimized for xcore.ai.

Usage
=====


.. code-block:: console

    $ xformer.py <tflite_input> <tflite_output>

*******
Options
*******

Positional Arguments
====================

.. option:: tflite_input
  
    Input .tflite file.


.. option:: tflite_output

    Output .tflite file.


Overall Options
===============

.. option:: -v, --verbose

    Set verbosity level. 
   
    ``-v`` : summary info of mutations;

    ``-vv``: detailed mutation and debug info.

.. option:: --minify

    Make the model smaller at the expense of readability. 
   
.. option:: -par, --num_threads <NUM_THREADS>

    Number of parallel threads for xcore.ai optimization. 

.. option:: --intermediates_path <INTERMEDIATES_PATH>

    Path to directory for storing intermediate models. If
    not given intermediate models will not be saved. If
    path doesn't exists, it will be created. Contents may
    be overwritten 

.. option:: --analyze

    Analyze the output model. A report is printed showing
    the runtime memory footprint of the model.

.. option:: --version

    Print version string. 

.. option:: -h, --help

    Print help message. 

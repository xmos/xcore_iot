
#######################
Installing the AI Tools
#######################

********
Overview
********

The AI Tools are a collection of Python scripts, tools and libraries to convert TensorFlow Lite for Microcontroller models to a format targeting accelerated operations on the xcore.ai platform.

************
Installation
************

System Requirements
===================

Make sure your system meets the minumum :ref:`system requirements <sdk-system-requirements-label>` and that you have installed all necessary :ref:`prerequisites <sdk-prerequisites-label>`.

In addition, `Python 3.8 <https://www.python.org/downloads/>`_ + is required.


Set up Virtual Environment
==========================

While not required, we recommend you setup an `Anaconda <https://www.anaconda.com/products/individual/>`_ or other Python virtual environment before installing the AI Tools.

If necessary, download and follow Anaconda's installation instructions.

Run the following command to create a Conda environment:

.. code-block:: console

    $ conda create --prefix xcore_sdk_venv python=3.8

Run the following command to activate the Conda environment:

.. code-block:: console

    $ conda activate xcore_sdk_venv

Install AI Tools
================

The following commands will install all required libraries.

.. code-block:: console

    $ cd tools/ai
    $ git clone git@github.com:xmos/ai_tools.git
    $ git checkout cb81a22
    $ git submodule update --init --recursive
    $ bash install.sh
    $ python -m pip install --upgrade pip
    $ pip install -r requirements.txt

The following commands will add the AI Tool scripts to your PATH:

.. code-block:: console

    $ source tools/ai/SetEnv

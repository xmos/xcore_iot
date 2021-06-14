
#######################
Installing the AI Tools
#######################

********
Overview
********

The AI Tools are a collection of Python scripts, tools and libraries to convert TensorFlow Lite for Microcontroller models to a format targetting accelerated operations on the xcore.ai platform.

************
Installation
************

System Requirements
===================

Make sure your system meets the minumum :ref:`system requirements <sdk-system-requirements-label>` and that you have installed all necessary :ref:`prerequisites <sdk-prerequisites-label>`.

In addition, `Python 3.8 <https://www.python.org/downloads/>`_ + is required.


.. _ai_tools-setup-virtual-environment-label:

Set up Virtual Environment
==========================

While not required, we recommend you setup an `Anaconda <https://www.anaconda.com/products/individual/>`_ environment before installing the AI Tools.  If necessary, download and follow Anaconda's installation instructions.

To create a Conda environment

.. code-block:: console

    $ conda create --prefix xcore_sdk_venv python=3.8

.. _ai_tools-activate-virtual-environment-label:

To activate the environment

.. code-block:: console

    $ conda activate xcore_sdk_venv

.. note:: You may need to specify the fully-qualified path to your environment.

.. note:: You can also add an alias to your ``.profile`` or ``.bash_profile`` script to make activating the environment easier. Add a line similar to ``alias xcore_sdk_env='conda activate xcore_sdk_env'``.  The next time you launch a console, the command ``xcore_sdk_env`` will activate the environment.

Install AI Tools
================

The following command will install all required libraries.

.. code-block:: console

    $ ./tools/ai/install.sh

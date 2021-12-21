.. include:: ../../substitutions.rst

############
Build System
############

This document describes the `CMake <https://cmake.org/>`_-based build system used by the example applications in the XCore SDK.  The build system is designed so a user does not have to be an expert using CMake.  However, some familiarity with CMake is helpful.  You can familiarize yourself by reading the `Cmake Tutorial <https://cmake.org/cmake/help/latest/guide/tutorial/index.html>`_ or `CMake documentation <https://cmake.org/cmake/help/v3.20/>`_.  

********
Overview
********

An XCore SDK project can be seen as an integration of several modules. For example, for a FreeRTOS application that captures audio from PDM microphones and outputs it to a DAC, 
there could be the following modules:

- The SDK core modules (for debug prints, etc...)
- The FreeRTOS kernel
- Microphone array driver for audio samples
- |I2C| driver for configuring the DAC
- |I2S| driver for output to the DAC
- Application code tying it all together

When a project is compiled, the build system will gather all of the source code and include paths for all the configured modules.  

********
Concepts
********

A **project** is a directory that contains all the files and configuration to build a single application.

**modules** are pieces of standalone code which are compiled and linked into an application. Many modules are provided by the SDK itself, others may be sourced from other places.

**Target** is the hardware for which an application is built. 

The XCore SDK is not part of the project. Instead it is standalone, and linked to the project via the XCORE_SDK_PATH environment or CMake variable. This allows the SDK to be decoupled from your project.  The XTC toolchain for compilation is also not part of the project. The toolchain should be installed seperately.

**********************
Using the Build System
**********************

Below are some examples ``CMakeLists.txt`` files that should get you started using the SDK's build system.  In addition, the SDK's build system supports many configuration options.

.. toctree::
   :maxdepth: 2

   cmakelists
   cmake_variables

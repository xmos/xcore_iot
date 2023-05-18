.. _sdk-faq:

#############################
Frequently Asked Questions
#############################

*******
General
*******

=================
Submodule updates
=================

XCORE-IOT uses submodules.  If you have cloned the repository and later perform an update, it will sometimes also be necessary to update the submodules.  To update all submodules, run the following command

.. code-block:: console

    git submodule update --init --recursive

==================
Explorer Board 1v1
==================

The Explorer Board 2v0 is supported by default in all example applications.  However, it is possible to target the Explorer Board 1v1.
In the example application CMakeLists file, change this line:

.. code-block:: cmake

    set(APP_LINK_LIBRARIES
        rtos::bsp_config::xcore_ai_explorer
    )

to this:

.. code-block:: cmake

    set(APP_LINK_LIBRARIES
        rtos::bsp_config::xcore_ai_explorer_1V1
    )

************
Build Issues
************

========================
fatfs_mkimage: not found
========================

This issue occurs when the ``fatfs_mkimage`` utility cannot be found.  The most common cause for these issues are an incomplete installation of the XCORE-IOT.

Ensure that the host applications setup has been completed.  Verify that the ``fatfs_mkimage`` binary is installed to a location on PATH, or that the default application installation folder is added to PATH.  See the :ref:`sdk-installation` guide for more information on installing the host applications.

===============================================
xcc2clang.exe: error: no such file or directory
===============================================

Those strange characters at the beginning of the path are known as a byte-order mark (BOM). CMake adds them to the beginning of the response files it generates during the configure step. Why does it add them? Because the MSVC compiler toolchain requires them. However, some compiler toolchains, like ``gcc`` and ``xcc``, do not ignore the BOM. Why did CMake think the compiler toolchain was MSVC and not the XTC toolchain? Because of a bug in which certain versions of CMake and certain versions of Visual Studio do not play nice together. The good news is that this appears to have been addressed in CMake version 3.22.3. 

Update to CMake version 3.22.2 or newer.

********
FreeRTOS
********

See the :ref:`freertos-faq` or :ref:`freertos-common_issues`

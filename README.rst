RTOS Support Library
===============

Summary
-------

A software library that provides support for running an RTOS on xCORE. SMP RTOS support is also provided. Functionality is split between two sub-libraries, lib_rtos_support and lib_soc.
 
lib_rtos_support is a dependency of each RTOS port. It provides functionality that is common across all RTOS (both SMP and single core) ports to xCORE such as support for using interrupts and recursive hardware locks. It also provides an interrupt and RTOS safe version of (s)(n)printf.
 
lib_soc provides a system on chip like platform in which an RTOS application may run. It allows for separation of an RTOS application from the more hardware-like xCORE libraries that run on their own cores, and a mechanism for communication between them. Common peripheral devices, including i2c and Ethernet, are provided, as well as drivers to use them from an RTOS application. Currently only FreeRTOS is supported by the RTOS drivers.

Example FreeRTOS applications demonstrating how to use the library can be found in the examples folder.

All dependent libraries are included as git submodules. These can be obtained by cloning this repository with the following command::

     git clone --recurse-submodules git@github.com:xmos/lib_rtos_support.git

Or by running the following commands inside the repository::

     git submodule init
     git submodule update

An xTIMEcomposer workspace may be created in the root of this repository. All library projects, the FreeRTOS demo project found in FreeRTOS/FreeRTOS/Demo/XCORE200_XCC/RTOSDemo, and the two example application projects may all be imported into the workspace.

Features
........


Resource Usage
..............

.. resusage::


Software version and dependencies
.................................

.. libdeps::

............................................


Related application notes
.........................


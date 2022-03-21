.. _sdk-system-requirements-label:

###################
System Requirements
###################

The XCore SDK is officially supported on the following platforms. Unofficial support is mentioned where appropriate.


.. tab:: Windows

   Windows 10 is supported.
   
   Windows users can also use the Windows Subsystem for Linux (WSL).  See `Windows Subsystem for Linux Installation Guide for Windows 10 <https://docs.microsoft.com/en-us/windows/wsl/install-win10>`__ to install WSL.

   The SDK should also work using other Windows GNU development environments like GNU Make, MinGW or Cygwin.

.. tab:: Mac

   Operating systems macOS 10.5 (Catalina) and newer are supported. Intel processors only.  Older operating systems are likely to also work, though they are not supported.

   A standard C/C++ compiler is required to build applications and libraries on the host PC.  Mac users may use the Xcode command line tools.

.. tab:: Linux

   Many modern Linux distros including Fedora, Ubuntu, CentOS & Debian are supported.

.. _sdk-prerequisites-label:

*************
Prerequisites
*************

`XTC Tools 15.0.6 <https://www.xmos.com/software/tools/>`_ or newer and `CMake 3.21 <https://cmake.org/download/>`_ or newer are required for building the example applications.  If necessary, download and follow the installation instructions for those components.

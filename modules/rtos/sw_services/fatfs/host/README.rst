===================================
FAT File System Image Creation Tool
===================================

This is the FAT file system image creation tool. This tool creates a FAT filesystem image that is populated with the contents of the specified directory. All filenames must be in 8.3 format.


*****
Setup
*****

This application requires the xcore_sdk.

Set the environment variable XCORE_SDK_PATH to the root of the xcore_sdk.

.. tab:: Linux and MacOS

    .. code-block:: console

        $ export XCORE_SDK_PATH=/path/to/sdk
        
.. tab:: Windows

    .. code-block:: command prompt
    
        > set XCORE_SDK_PATH=C:\path\to\sdk


************************
Building the Application
************************

To build this application, run the following commands:

.. tab:: Linux and MacOS

    .. code-block:: console
    
        $ cmake -B build
        $ cd build
        $ make -j

Note: Windows users must run the x86 native tools command prompt from Visual Studio

.. tab:: Windows

    .. code-block:: x86 native tools command prompt
    
        > cmake -G "NMake Makefiles" -B build
        > cd build
        > nmake


*******************
Verify Installation
*******************

To verify this application has built, ascend from the build/ directory to its parent and run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ fatfs_mkimage.exe --help
        
.. tab:: Windows

    .. code-block:: x86 native tools command prompt
    
        > fatfs_mkimage --help
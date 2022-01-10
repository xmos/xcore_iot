===================================
FAT File System Image Creation Tool
===================================

This is the FAT file system image creation tool. This tool creates a FAT filesystem image that is populated with the contents of the specified directory. All filenames must be in 8.3 format.


************************
Building the Application
************************

This application is typically built and installed from tools/install.  

However, if you are modifying the application, it is possible to build the project using CMake. To build this application, run the following commands:

.. tab:: Linux and MacOS

    .. code-block:: console
    
        $ cmake -B build
        $ cd build
        $ make -j

Note: You may need to run the ``make -j`` command as ``sudo``.  

Note: Windows users must run the x86 native tools command prompt from Visual Studio

.. tab:: Windows

    .. code-block:: x86 native tools command prompt
    
        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake

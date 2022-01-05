===================================
FAT File System Image Creation Tool
===================================

This is the FAT file system image creation tool. This tool creates a FAT filesystem image that is populated with the contents of the specified directory. All filenames must be in 8.3 format.


*****************************
Building the Host Application
*****************************

To build this application, run the following command:

Note: Linux users require proper permissions for installation to the /opt/ directory.

.. tab:: Linux and MacOS

    .. code-block:: console
    
        $ ./install_host_app.sh

Note: Windows users must run the x86 native tools command prompt from Visual Studio

.. tab:: Windows

    .. code-block:: x86 native tools command prompt
    
        > install_host_app.bat


*******************
Verify Installation
*******************

To verify this application has built, run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ fatfs_mkimage.exe --help
        
.. tab:: Windows

    .. code-block:: x86 native tools command prompt
    
        > fatfs_mkimage.exe --help
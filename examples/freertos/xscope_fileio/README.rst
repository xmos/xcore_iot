################################
FreeRTOS XSCOPE File I/O Example
################################

This FreeRTOS example application reads a file from the host over an XSCOPE server, propagates the data through multiple threads across both tiles, and then writes to a host file, also over an XSCOPE server.

.. image:: images/functional_diagram.png
    :align: left

******************
Supported Hardware
******************

This example is supported on the XCORE-AI-EXPLORER_2V0 board.

*********************
Building the Firmware
*********************

Run the following commands in the root folder to build the embedded application using the XTC Toolchain:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_xscope_fileio

.. tab:: Windows

    Before building the embedded application, you may need to add the path to nmake to your PATH variable.

    .. code-block:: console

        set PATH=%PATH%;<path-to-nmake>

    To build the embedded application:

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_freertos_xscope_fileio

*********************
Building the Host App
*********************

Run the following commands in the root folder to build the host application using your native x86 Toolchain:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build_host
        cd build_host
        make xscope_host_endpoint
        make install

.. tab:: Windows

    Before building the host application, you will need to add the path to the XTC Tools to your environment.
    
    .. code-block:: console

        set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

    Then build the host application:

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build_host
        cd build_host
        nmake xscope_host_endpoint
        nmake install

Note: Permissions may be required to install the host applications.

The host application, `xscope_host_endpoint`, will be installed at `applications/hark`, and may be moved if desired.

********************
Running the Firmware
********************

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_freertos_xscope_fileio

.. tab:: Windows

    .. code-block:: console

        nmake run_example_freertos_xscope_fileio

In a second console, run the host xscope server:

.. tab:: Linux and Mac

    .. code-block:: console

        ./xscope_host_endpoint 12345

.. tab:: Windows

    Before running the host application, you may need to add the location of the `xscope_endpoint.dll` to your PATH.

    .. code-block:: console

        set PATH=%PATH%;<path-to-xscope-endpoint-dll>

    Then run the host application:

    .. code-block:: console

       xscope_host_endpoint.exe 12345

#################
Graphic Equaliser
#################

This example application implements the `Graphic Equaliser <https://www.dsprelated.com/showcode/169.php>`__ model.  The graphic equaliser model will take a wav file as input and perform a signal processing to get an output wav file.

This example demonstrates how to receive input data using `xscope`.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_bare_metal_graphic_equaliser

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_bare_metal_graphic_equaliser

********************
Running the firmware
********************

Running with hardware

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_example_bare_metal_graphic_equaliser

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake run_example_bare_metal_graphic_equaliser

Running with simulator

.. code-block:: console

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make xsim_example_bare_metal_graphic_equaliser

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake xsim_example_bare_metal_graphic_equaliser

The firmware will now wait until a data is sent from a host application. Test audio files can be sent to the firmware using `xscope`.  Only 1 channel wav format is supported.  The `host_app.py` script requires Python.  Ensure you have installed Python 3 and the XCore SDK Python requirements.

Sending a test audio file to the xcore.ai Explorer board using `xscope`. The `host_app.py` script can be found in the application directory:

.. code-block:: console

    $ ./host_app.py path/to/input_wav path/to/output_wav


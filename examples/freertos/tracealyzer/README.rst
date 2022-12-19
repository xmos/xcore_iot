################################
FreeRTOS Tracealyzer Example
################################

This is a simple multi-tile FreeRTOS example application illustrating how to use
FreeRTOS' trace functionality with Percepio's Tracealyzer. The application
illustrates a timeout issue in an example state machine which can be
visualized/diagnosed with Tracealyzer. In the absence of Tracealyzer, it is
possible to define another trace implementation, see `FreeRTOS Trace Macros`_
documentation for more details. For such instances, an ASCII trace
implementation is available as a good starting point. This can be enabled by
changing the trace mode define in the cmake file to:
`USE_TRACE_MODE=TRACE_MODE_XSCOPE_ASCII`.

The application starts the FreeRTOS scheduler running on both `tile[0]` and
`tile[1]`. `tile[0]` has 11 tasks, whereas `tile[1]` has only 1 task running.
Both `tile[0]` and `tile[1]` share the same logic for a "hello" task which
prints a message every second. The other 10 `tile[0]` tasks serve to demonstrate
an issue that can be introduced on command by the user by interacting with the
buttons on the xCORE.AI Explorer board. Pressing button 1 will increase a
counter up to a maximum value of 8 (while button 0 decreases this counter down
to a minimum value of 0). This value affects how many `subprocess` tasks
sequentially interrupt the main `process` task. The main `process` task monitors
timing while in its `RUN` state. If it detects an interruption greater than or
equal to a configured threshold, the `process` will momentarily transition to
a `timeout` state. Pressing Button 1 four or more consecutive times should
result in this timeout event. Using tools such as Tracealyzer reduces the effort
involved in diagnosing multi-core/task applications.

****************************
Limitations and Known Issues
****************************

The following are the currently known issues/limitations for this example:

- Tracing is performed on a single tile at a time. In this example, Tracealyzer
  is setup on `tile[0]`.
- Tracealyzer's snapshot mode is not supported.
- It may be necessary to disable certain trace events (see `trcConfig.h`),
  limit user events (i.e. via xTracePrint), or disable additional xSCOPE probes
  to reduce the bandwidth requirements over xSCOPE. In some cases the
  application may exit prematurely or drop trace data when there are
  exceptionally high number of trace events being recorded. This behavior may be
  attributed to the host PC's USB controller or general performance factors
  regarding the offloading of trace data from the XTAG. In such cases,
  xscope2psf will log a "missing events" warning.

*********************
Building the Host App
*********************

Run the following commands in the root folder to build the host application
using your native x86 Toolchain:

.. note::

    Permissions may be required to install the host applications.

Linux or Mac
------------

    .. code-block:: console

        cmake -B build_host
        cd build_host
        make xscope2psf
        make install

The host application, `xscope2psf`, will be installed at `/opt/xmos/SDK/<sdk version>/bin/`,
and may be moved if desired.

Windows
-------

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build_host
        cd build_host
        nmake xscope2psf
        nmake install

The host application, `xscope2psf.exe`, will be install at `%USERPROFILE%\.xmos\SDK\<sdk version>\bin\\`,
and may be moved if desired.

The instructions that follow will assume that the path of this binary has been
added to your `PATH` variable or the binary has been copied to the current
directory.

*********************
Building the Firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

Linux or Mac
------------

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_tracealyzer

Windows
-------

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_freertos_tracealyzer

*********************
Running the Firmware
*********************

From the xcore_sdk build folder run:

Linux or Mac
------------

    .. code-block:: console

        make run_xscope_to_file_example_freertos_tracealyzer

Windows
-------

    .. code-block:: console

        nmake run_xscope_to_file_example_freertos_tracealyzer

If successful, the console should have printed a subset of messages similar to
the following:

    .. code-block:: console

        Hello task running from tile 1 on core 4
        Entered subprocess task (7) on core 3
        Entered subprocess task (6) on core 4
        Entered subprocess task (5) on core 5
        Entered subprocess task (4) on core 0
        Entered subprocess task (3) on core 2
        Entered subprocess task (2) on core 3
        Entered subprocess task (1) on core 4
        Entered subprocess task (0) on core 5
        Entered main process on core 0
        Hello task running from tile 0 on core 2
        Entered gpio task on core 1
        Hello from tile 0
        Hello from tile 1
        Hello from tile 0
        Hello from tile 1

The LED behavior should be as follows:

- LED 0 should turn on while Button 0 is pressed.
- LED 1 should turn on while Button 0 is pressed.
- LED 2 should toggle when the main process enters the timeout state.
- LED 3 should toggle every 500ms.

There should also be two new files generated:

- freertos_trace.vcd
- freertos_trace.gtkw

*********************************
Generating a Tracealyzer PSF File
*********************************

With the previously generated `freertos_trace.vcd` file, from the xcore_sdk
build directory run:

    .. code-block:: console

        xscope2psf -v -i freertos_trace.vcd -o freertos_trace.psf

The output from this command should look similar to what is shown below:

    .. code-block:: console

        Opening input file ...
        Opening output file ...
        Processing file (Probe: 0) ...
        [PSF Header]
        - Format Version: 0x000A
        - Options: 0x00000000
        - Number of Cores: 6
        - Platform: FreeRTOS
        - Platform ID: 0x1AA1
        - Platform Config: 1.0 Patch 0
        - ISR Tail-Chaining Threshold: 0
        [PSF Timestamp]
        - Type: 1
        - Frequency: 100000000
        - Period: 100000
        - Wraparounds: 0
        - OS Tick Hz: 1000
        - Latest Timestamp: 0
        - OS Tick Count: 0
        End of file reached.
        Read 282879 lines.
        Processed 70714 events.
        Closing files ...
        Done.

Successful execution of this command will produce the Percepio Streaming Format
(PSF) file that can be opened in Tracealyzer for inspection.

************************************
Live Trace Visualization (streaming)
************************************

The previous steps illustrated a way to save a VCD trace to disk and post
process it. Alternatively, this workflow can be changed to visualize the trace
live. Two methods are currently available for this which will be discussed in
this section.

Before continuing, Tracealyzer must be configured to use the 'File System` as
the PSF streaming option. This can be configured via the following steps:

1. From the menubar in Tracealyzer, click `File` --> `Settings`
2. In the `Settings` window's left-hand menu tree, click `Project Settings`
   --> `PSF Streaming Settings`.
3. Under `Target Connection` select `File System`.
4. This setting will provide an option to specify a PSF file. Specify the
   `freertos_trace.psf` file that was previously generated.
5. Click `OK`.
6. From the menubar, click `Trace` --> `Open Live Stream Tool`.
7. This will open a new `Live Stream` window, in this window click `Connect`.

With the xrun/xgdb `example_freertos_tracealyzer.xe` and `xscope2psf`
applications still running, it should now be possible to click `Start Session`
and see the trace data live. Alternatively, the `Start` and `Stop` recording
button in the main window's left hand menu bar may be utilized for control.

.. note::

    The `Live Stream` window's reported `Event Rate` and `Data Rate` is useful
    when optimizing xscope bandwidth utilization and to determine if it is
    necessary to limit the frequency or types of events being recorded. A
    `Data Rate` versus time graph can be shown in this window via the menubar's
    `View` --> `Data Rate` option.


Using --xscope-file
-------------------

From the xcore_sdk build folder run:

1. Start the application:

    .. code-block:: console

        xrun --xscope-file freertos_trace example_freertos_tracealyzer.xe

2. Start the PSF file generation process:

    .. code-block:: console

        xscope2psf -v -s -i freertos_trace.vcd -o freertos_trace.psf

As the VCD file is being written to (via xscope), xscope2psf will produce status
updates on the number of lines processed and how many events have been written
to the PSF file. The console output will look similar to the following:

    .. code-block:: console

        Opening input file ...
        Opening output file ...
        Processing file (Probe: 0) ...
        [PSF Header]
        - Format Version: 0x000A
        - Options: 0x00000000
        - Number of Cores: 6
        - Platform: FreeRTOS
        - Platform ID: 0x1AA1
        - Platform Config: 1.0 Patch 0
        - ISR Tail-Chaining Threshold: 0
        [PSF Timestamp]
        - Type: 1
        - Frequency: 100000000
        - Period: 100000
        - Wraparounds: 0
        - OS Tick Hz: 1000
        - Latest Timestamp: 0
        - OS Tick Count: 0
        [STREAM STATUS]
        - Read 33027 lines
        - Processed 8251 events
        [STREAM STATUS]
        - Read 41359 lines
        - Processed 10334 events
        [STREAM STATUS]
        - Read 47431 lines
        - Processed 11852 events
        [STREAM STATUS]
        - Read 56771 lines
        - Processed 14187 events

Using --xscope-port
-------------------

1. Start the application:

    .. code-block:: console

        xrun --xscope-port localhost:10234 example_freertos_tracealyzer.xe

2. Start the PSF file generation process:

    .. code-block:: console
        xscope2psf -v -I localhost:10234 -o freertos_trace.psf

As record data is sent to xscope2psf it will produce status updates on the
number of events written to the PSF file. The console output will look similar
to the following:

    .. code-block:: console

        Configuring xscope callbacks ...
        Opening output file ...
        Connecting to xscope (Probe: 0, Host: localhost, Port: 10234) ...
        [REGISTERED] Probe ID: 0, Name: 'freertos_trace'
        [PSF Header]
        - Format Version: 0x000A
        - Options: 0x00000000
        - Number of Cores: 6
        - Platform: FreeRTOS
        - Platform ID: 0x1AA1
        - Platform Config: 1.0 Patch 0
        - ISR Tail-Chaining Threshold: 0
        [PSF Timestamp]
        - Type: 1
        - Frequency: 100000000
        - Period: 100000
        - Wraparounds: 0
        - OS Tick Hz: 1000
        - Latest Timestamp: 0
        - OS Tick Count: 0
        [STREAM STATUS]
        - Processed 162 events
        [STREAM STATUS]
        - Processed 1585 events
        [STREAM STATUS]
        - Processed 3902 events
        [STREAM STATUS]
        - Processed 5288 events

In this case the target application's `printf` output will not be present in
either xrun/xgdb or xscope2psf (while xscope2psf is connected). This output can
be emitted on xscope2psf by providing the `--print-endpoint` option. It is
recommended to use the `-p` and `-v` options separately as the current
implementation of this utility does not provide any measures to ensure the
target's printf log entries are not interrupted by the regular stream status
reporting.

.. _FreeRTOS Trace Macros: https://www.freertos.org/rtos-trace-macros.html
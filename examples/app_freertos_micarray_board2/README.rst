How to use the app_freertos_micarray_board2 application
========================================

.. version:: 0.5.0

Summary
-------

This application note shows how to use the RTOS support library to make
FreeRTOS applications targetting xCORE. The application uses I2C, I2S,
ethernet, mic array, and gpio devices.

The app_freertos_micarray_board and app_freertos_micarray_board2 applications,
only differ in placement of the FreeRTOS kernel.  In app_freertos_micarray_board,
the FreeRTOS kernel is on tile 0.  In app_freertos_micarray_board2, the FreeRTOS
kernel is on tile 1.  This is to demonstrate how to use the on-tile and inter-tile
devices.

The application creates a single stage audio pipeline, which applies a
variable gain.  The output audio is sent to the DAC and can be listened
to via the 3.5mm audio jack.  Alternatively, the audio can be streamed
over TCP as raw PCM to be played back on the host device.  The audio gain
can be adjusted in two ways.  Firstly, through a UDP command line interface,
which additionally implements a portion of the FreeRTOS Plus CLI demo.  
Secondly, through gpio, where button A is volume up, button B is volume down,
button C is mute/unmute, and button D is default volume.

Resource usage
............................
Constraint check for tile[0]:
  Cores available:            8,   used:          4 .  OKAY
  Timers available:          10,   used:          4 .  OKAY
  Chanends available:        32,   used:          6 .  OKAY
  Memory available:       262144,   used:      27948 .  OKAY
    (Stack: 1460, Code: 12132, Data: 14356)
Constraints checks PASSED.
Constraint check for tile[1]:
  Cores available:            8,   used:          8 .  OKAY
  Timers available:          10,   used:          9 .  OKAY
  Chanends available:        32,   used:         19 .  OKAY
  Memory available:       262144,   used:      238740 .  OKAY
    (Stack: 25860, Code: 65768, Data: 147112)
Constraints checks PASSED.

Required tools and libraries
............................

* xTIMEcomposer Tools - Version 14.4.1 
* XMOS FreeRTOS library - Version 10.2.1

Required hardware
.................

This application note is designed to run on the XK-USB-MIC-UF216 development kit.

Prerequisites
.............

  - This document assumes familiarity with the XMOS xCORE
    architecture, FreeRTOS, I2C, I2S, the XMOS tool chain, and the xC language.

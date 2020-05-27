How to use the app_freertos_micarray_board application
========================================

.. version:: 0.5.0

Summary
-------

This example application shows how to use the lib_soc and FreeRTOS libraries to
make FreeRTOS applications targetting xCORE. The application uses I2C, I2S,
Ethernet, mic array, and GPIO devices. These devices are instantiated in the
"bitstream_src" folder. The FreeRTOS application which communicates with these
devices is contained within the "src" folder.

The app_freertos_micarray_board and app_freertos_micarray_board2 applications
only differ in the placement of the FreeRTOS kernel. In app_freertos_micarray_board,
the FreeRTOS kernel is on tile 0. In app_freertos_micarray_board2, the FreeRTOS
kernel is on tile 1. This is to demonstrate how to connect FreeRTOS to all the
peripherals regardless of whether they are on the same tile or not.

The application creates a single stage audio pipeline which applies a variable
gain. The output audio is sent to the DAC and can be listened to via the 3.5mm
audio jack. Additionally, the audio can be streamed out over TCP as raw PCM which
may be played back on the host device. The audio gain can be adjusted in two ways.
Firstly, through a UDP command line interface which is implemented with the
FreeRTOS Plus CLI library. Secondly, through GPIO, where button A is volume up,
button B is volume down, button C is mute/unmute, and button D is default volume.

The application also runs a TCP throughput test server. When a client connects it
sends 100 MiB of data as fast as it can and then disconnects.

The script example_host.sh can be used to stream the audio over TCP, connect to the
command line interface, and to run the throughput test:
 - example_host.sh -n IP connects to the audio stream server and plays the audio
 - example_host.sh -t IP connects to the throughput test server and shows the current
   and average throughput.
 - example_host.sh -u IP connects to the command line interface. Once connected type
   help for a list of supported commands.
   
The IP of the board is obtained via DHCP and is printed out over xscope I/O.

Resource usage
............................
Constraint check for tile[0]:
  Cores available:            8,   used:          6 .  OKAY
  Timers available:          10,   used:          6 .  OKAY
  Chanends available:        32,   used:          8 .  OKAY
  Memory available:       262144,   used:      220648 .  OKAY
    (Stack: 2932, Code: 59160, Data: 158556)
Constraints checks PASSED.
Constraint check for tile[1]:
  Cores available:            8,   used:          7 .  OKAY
  Timers available:          10,   used:          8 .  OKAY
  Chanends available:        32,   used:         13 .  OKAY
  Memory available:       262144,   used:      48816 .  OKAY
    (Stack: 24500, Code: 18528, Data: 5788)
Constraints checks PASSED.

Required tools and libraries
............................
 * xTIMEcomposer Tools - Version 14.4.1 
 * XMOS FreeRTOS library - Version 10.2.1
 * The pv utility to run the throughput test with example_host.sh.

Required hardware
.................
This application note is designed to run on the XK-USB-MIC-UF216 development kit.

Prerequisites
.............
 * This document assumes familiarity with the XMOS xCORE
   architecture, FreeRTOS, I2C, I2S, the XMOS tool chain, and the xC language.


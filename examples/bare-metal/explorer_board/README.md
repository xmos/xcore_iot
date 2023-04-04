# Explorer Board

This example application demonstrates various capabilities of the Explorer board.

The example consists of pdm_mics to a simple audio processing pipeline which
applies a variable gain.  Pressing button 0 will increase the gain.  Pressing
button 1 will decrease the gain.  The processed audio is sent to the DAC.

When button 0 is pressed, LED 0 will be lit.  When button 1 is pressed, LED 1
will be lit.  When the gain adjusted audio passes a frame power threshold, LED 2
will be lit.  Lastly, LED 3 will blink periodically.

Additionally, the example demonstrates a simple flash, UART loopback and SPI setup.

## CMake Targets

The following CMake targets are provided:

- example_bare_metal_explorer_board
- run_example_bare_metal_explorer_board
- debug_example_bare_metal_explorer_board

## Deploying the Firmware

See the Programming Guide for information on building and running the application.

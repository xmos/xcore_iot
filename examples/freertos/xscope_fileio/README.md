# XSCOPE File I/O

This example application reads a WAV file from the host over an XSCOPE server, propagates the data through multiple threads across both tiles, and then writes the output to a WAV file on the host PC, also over an XSCOPE server.

## CMake Targets

The following CMake targets are provided:

- example_freertos_xscope_fileio
- run_example_freertos_xscope_fileio

## Deploying the Firmware

See the Programming Guide for information on building and running the application.
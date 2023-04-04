# Audio Mux Example Design

This example application can be configured for onboard mic, usb audio, or i2s input.  Outputs are usb audio and i2s.  No DSP is performed on the audio, but the example contains an empty 2 tile pipeline skeleton for a user to populate.

## CMake Targets

The following CMake targets are provided:

- example_audio_mux
- run_example_audio_mux
- debug_example_audio_mux

## Deploying the Firmware

See the Programming Guide for information on building and running the application.

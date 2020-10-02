# Explorer Board example application

This example application demonstrates various capabilities of the Explorer board using FreeRTOS.  The example uses lib_soc and various libraries to build FreeRTOS applications targetting xCORE.  The application uses I2C, I2S, SPI, flash, mic array, and GPIO devices.  These devices are instantiated in the "bitstream_src" folder.  The FreeRTOS application which sets up drivers to communicate with these devices is contained within the "src" folder.

The application places FreeRTOS on tile 0.  The peripherals are placed on tile 0 and tile 1 to demonstrate how to connect devices to FreeRTOS regardless of tile placement.

The FreeRTOS application creates a single stage audio pipeline which applies a variable
gain. The output audio is sent to the DAC and can be listened to via the 3.5mm
audio jack. Additionally, the audio can be streamed out over TCP as raw PCM which
may be played back on the host device. The audio gain can be adjusted in two ways.
Firstly, through a UDP command line interface which is implemented with the
FreeRTOS Plus CLI library. Secondly, through GPIO, where button A is volume up and
button B is volume down.

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

## Prerequisites for building

[XMOS Toolchain 15.0.1](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.14 or newer.

Set environment variable for the XMOS AIoT SDK:

    > export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

## Building for xCORE

Make a directory for the build.

    > mkdir build
    > cd build

Run cmake:

    > cmake ../ -DBOARD=XCORE-AI-EXPLORER
    > make

If you plan on running the demo where the Explorer Board connects to a host side echo server, modify src/app_conf.h to your host computer's IP address.

    /* Echo demo defines */
    #define appconfECHO_IP_ADDR_OCTET_0    	10
    #define appconfECHO_IP_ADDR_OCTET_1    	0
    #define appconfECHO_IP_ADDR_OCTET_2    	0
    #define appconfECHO_IP_ADDR_OCTET_3    	253

*Note: Your host computer and the developer kit need to be on the same WiFi network.*

## Filesystem setup
Before the demo can be run, the filesystem must be configured and flashed.

Note, macOS users will need to install `dosfstools`.

    > brew install dosfstools

To create certificates for the demos run:

    > cd filesystem_support
    > ./gen_demo_certs.sh

If certificates have previously been created run:

    > ./flash_image.sh

The script will prompt you for WiFi credentials:

    Enter the WiFi network SSID:
    Enter the WiFi network password:
    Enter the security (0=open, 1=WEP, 2=WPA):
    Add another WiFi network? (y/n):

*Note: Once a wifi profile has been created it will automatically be used.  If you need to change the profile delete networks.dat and rerun flash_image.sh*

## Running the Demo
To run the demo navigate to the bin folder and use the command:

    > xrun --xscope explorer_board.xe

### Host FREERTOS_INVALID_SOCKET

In a second console you can run the example_host script to demo various actions.

#### Thruput Test

The thruput test sends 1 MiB of data to test network transmit speed.

    ./example_host.sh -t [board IP addr]

#### Stream audio

This will stream audio from the audio pipeline to the host computer.  This demo requires aplay on the host machine.

    ./example_host.sh -n [board IP addr] 16000

#### UDP CLI

Connects to the FreeRTOS-Plus UPD based CLI demo.  Send "help" for information on available commands.

    ./example_host.sh -u [board IP addr]

#### Echo Server

Connects to the board hosted echo server using TLS.  Type a message and press enter to send.  The board will echo the payload back to the host.

    ./example_host.sh -c [board IP addr]

#### Echo Client

The board will try to connect to a hosted echo server using TLS.  When this command is run, the host will act as an echo server.  When the board connects, it will send the message HELLO WORLD, and receive the host response.

    ./example_host.sh -e

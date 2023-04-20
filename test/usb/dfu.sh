#!/usr/bin/env bash

function trace_help() {
    echo "Options:"
    echo "--upgrade / -u : Create upgrade image"
    echo "--flash / -f : Flash factory image"
    return
}

if [ $# == 1 ]
then
    if [ "$1" == "--help" ] || [ "$1" == "-h" ]
    then
        trace_help
    elif [ "$1" == "--upgrade" ] || [ "$1" == "-u" ]
    then
        xflash --factory-version 15.0 --upgrade 0 bin/usb.xe -o upgrade.bin
        echo "Created upgrade.bin"
    elif [ "$1" == "--flash" ] || [ "$1" == "-f" ]
    then
        xflash --quad-spi-clock 50MHz --factory bin/usb.xe --boot-partition-size 0x400000
    fi
else
    echo "Error! --help or -h for help"
fi

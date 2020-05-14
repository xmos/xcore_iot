#!/usr/bin/env bash

function trace_help() {
    echo "Options:"
    echo "--ncaplay / -n IP RATE: Connect to a stream over TCP and pipe into aplay"
    echo "--udpcli  / -u IP : Connect to CLI"
    echo "--thruput / -t IP : Run the throughput test"
    return
}

if [ $# == 1 ]
then
    if [ "$1" == "--help" ] || [ "$1" == "-h" ]
    then
        trace_help
    fi
elif [ $# == 2 ]
then
    if [ "$1" == "--udpcli" ] || [ "$1" == "-u" ]
    then
        ncat -u $2 5432
    elif [ "$1" == "--thruput" ] || [ "$1" == "-t" ]
    then
        ncat --recv-only $2 10000 | pv > /dev/null
    fi
elif [ $# == 3 ]
then
    if [ "$1" == "--ncaplay" ] || [ "$1" == "-n" ]
    then
        ncat --recv-only $2 54321 | aplay --format=S32_LE --rate=$3 --file-type=raw --buffer-size=14000
    fi
else
    echo "Error! --help or -h for help"
fi


#!/usr/bin/env bash

function trace_help() {
    echo "Options:"
    echo "--ncaplay / -n IP RATE: Connect to a stream over TCP and pipe into aplay"
    echo "--playto / -p IP FILENAME : Send raw signed int 32, 16kHz audio"
    echo "--udpcli  / -u IP : Connect to CLI"
    echo "--thruput / -t IP : Run the throughput test"
    echo "--http / -ht IP : Send a generic GET request"
    echo "--tlsechosrv / -e : Run an echo server with TLS"
    echo "--tlsechocli / -c : Connect to an echo server with TLS"
    return
}

if [ $# == 1 ]
then
    if [ "$1" == "--help" ] || [ "$1" == "-h" ]
    then
        trace_help
    elif [ "$1" == "--tlsechosrv" ] || [ "$1" == "-e" ]
    then
        ncat -e /bin/cat -k -4 -l 25565 --ssl --ssl-cert ./filesystem_support/echo_client_certs/server.pem  --ssl-key ./filesystem_support/echo_client_certs/server.key
    fi
elif [ $# == 2 ]
then
    if [ "$1" == "--udpcli" ] || [ "$1" == "-u" ]
    then
        ncat -u $2 5432
    elif [ "$1" == "--thruput" ] || [ "$1" == "-t" ]
    then
        ncat --recv-only $2 10000 | pv > /dev/null
    elif [ "$1" == "--tlsechocli" ] || [ "$1" == "-c" ]
    then
        ncat $2 7777 --ssl --ssl-cert ./filesystem_support/board_server_certs/client.pem --ssl-key ./filesystem_support/board_server_certs/client.key
    elif [ "$1" == "--http" ] || [ "$1" == "-ht" ]
    then
        echo "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc $2 80
    fi
elif [ $# == 3 ]
then
    if [ "$1" == "--ncaplay" ] || [ "$1" == "-n" ]
    then
        ncat --recv-only $2 54321 | aplay  --format=S32_LE --rate=$3 --file-type=raw --buffer-size=14000
    elif [ "$1" == "--playto" ] || [ "$1" == "-p" ]
    then
        cat $3 | ncat $2 12345
    fi
else
    echo "Error! --help or -h for help"
fi

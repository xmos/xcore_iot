#!/bin/bash

trap shutdown SIGTERM SIGINT

function shutdown() {
	killall xrun
	killall xgdb
	exit 0
}

xrun --xscope $1 &

while :
do
	sleep 1
done


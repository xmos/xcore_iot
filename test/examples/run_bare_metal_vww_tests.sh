#!/bin/bash
set -e

APPLICATION="example_bare_metal_vww"

if [ ! -z "$1" ]
then
    ADAPTER_ID="--adapter-id $1"
fi

(xrun --xscope $ADAPTER_ID $APPLICATION.xe 2>&1 | tee $APPLICATION.log)


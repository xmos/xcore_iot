#!/bin/bash
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

echo "****************"
echo "* Run Host     *"
echo "****************"
arecord testing/output.wav -D hw:CARD=DRIVERTEST,DEV=0 -f S16_LE -c 2 -r 16000 -d 10 &
aplay sine500hz.wav -D hw:CARD=DRIVERTEST,DEV=0

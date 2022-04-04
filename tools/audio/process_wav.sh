#!/bin/bash
set -e

help()
{
   echo "Voice reference design wav file processor"
   echo
   echo "Syntax: process_wav.sh [-c|h] to_device.wav from_device.wav"
   echo "options:"
   echo "h     Print this Help."
   echo "c     Number of channels in input wav"
   echo
}

# flag arguments
while getopts c:h option
do
    case "${option}" in
        c) CHANNELS=${OPTARG};;
        h) help
           exit;;
    esac
done

# positional arguments
INPUT_FILE=${@:$OPTIND:1}
OUTPUT_FILE=${@:$OPTIND+1:1}

# determine driver & device
uname=`uname`
if [[ "$uname" == 'Linux' ]]; then
    DEVICE_DRIVER="alsa"
    DEVICE_NAME="hw:CARD=Avona Voice Reference Design,DEV=0"
elif [[ "$uname" == 'Darwin' ]]; then
    DEVICE_DRIVER="coreaudio"
    DEVICE_NAME="Avona Voice Reference Design"
fi

# determine input remix pattern
#  the test vector input channel order is: Mic 1, Mic 0, Ref L, Ref R
#  NOTE: 3x10 output channel order is: Ref L, Ref R, Mic 1, Mic 0, ASR, Comms
#        Avona's output channel order is: ASR, Comms, Ref L, Ref R, Mic 0, Mic 1
if [[ "$CHANNELS" == 1 ]]; then # reference-less test vector
    # file only has 1 microphone channel
    #   need to insert 2 silent reference channels and repeat microphone channel
    REMIX_PATTERN="remix 0 0 1 1"
elif [[ "$CHANNELS" == 2 ]]; then # reference-less test vector
    # file only has microphone channels
    #   need to insert 2 silent reference channels
    REMIX_PATTERN="remix 0 0 2 1"
elif [[ "$CHANNELS" == 4 ]]; then # standard test vector
    REMIX_PATTERN="remix 3 4 2 1"
elif [[ "$CHANNELS" == 6 ]]; then  # assuming test vector from Avona
    REMIX_PATTERN="remix 3 4 5 6"
else
    REMIX_PATTERN=""
fi

# call sox pipelines
SOX_PLAY_OPTS="--buffer=65536 --rate=16000 --bits=16 --encoding=signed-integer --endian=little --no-dither"
SOX_REC_OPTS="--buffer=65536 --channels=6 --rate=16000 --bits=16 --encoding=signed-integer --endian=little --no-dither"

#set -x
sox -t $DEVICE_DRIVER "$DEVICE_NAME" $SOX_REC_OPTS -t wav $OUTPUT_FILE &

#sleep 1 # need to wait a bit

sox $INPUT_FILE $SOX_PLAY_OPTS -t wav - $REMIX_PATTERN | sox -t wav - -t $DEVICE_DRIVER "$DEVICE_NAME"

pkill -P $$
wait # to die

#!/bin/bash
set -e

# setup configuraitons
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "path/to/application BOARD"
    applications=(
        "examples/bare-metal/hello_world XCORE-AI-EXPLORER"
        "examples/bare-metal/cifar10 XCORE-AI-EXPLORER"
        "examples/bare-metal/hotdog_not_hotdog XCORE-AI-EXPLORER"
        "examples/bare-metal/visual_wake_words XCORE-AI-EXPLORER"
        "examples/bare-metal/micro_speech XCORE-AI-EXPLORER"
        "examples/bare-metal/micro_speech OSPREY-BOARD"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        "examples/bare-metal/visual_wake_words XCORE-AI-EXPLORER"
    )
else 
    echo "Argument $1 not a supported configuration!"
    exit
fi

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    application="${FIELDS[0]}"
    board="${FIELDS[1]}"
    echo '******************************************************'
    echo '* Building' ${application} 'for' ${board}
    echo '******************************************************'

    (cd ${application}; rm -rf build_${board})
    (cd ${application}; mkdir -p build_${board})
    (cd ${application}/build_${board}; cmake ../ -DBOARD=${board}; make -j install)
done

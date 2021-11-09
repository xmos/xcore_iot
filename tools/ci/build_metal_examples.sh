#!/bin/bash
set -e

source ${XCORE_SDK_PATH}/tools/ci/helper_functions.sh

function readable_run {
    if output=$("$@" 2>&1); then
        echo "'$@' completed successfully at $(date)"
    else
        echo "$output"
    fi    
}

# setup configuraitons
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "path/to/application BOARD"
    applications=(
        "examples/bare-metal/explorer_board XCORE-AI-EXPLORER"
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
        "examples/bare-metal/explorer_board XCORE-AI-EXPLORER"
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
    path="${XCORE_SDK_PATH}/${application}"
    echo '******************************************************'
    echo '* Building' ${application} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DBOARD=${board}; log_errors make -j install)
done

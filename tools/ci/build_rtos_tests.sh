#!/bin/bash
set -e

# setup configuraitons
if [ -z "$1" ] || [ "$1" == "all" ]
then
    # row format is: "path/to/application BOARD"
    applications=(
        "test/rtos_drivers/hil XCORE-AI-EXPLORER"
        "test/rtos_drivers/hil_add XCORE-AI-EXPLORER"
        "test/rtos_drivers/usb XCORE-AI-EXPLORER"
        "test/rtos_drivers/wifi XCORE-AI-EXPLORER"
    )
elif [ "$1" == "smoke" ]
then
    applications=(
        #"test/rtos_drivers/hil XCORE-AI-EXPLORER"
        "test/rtos_drivers/wifi XCORE-AI-EXPLORER"
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

    (cd ${application}; make distclean)
    (cd ${application}; make -j BOARD=${board})
done

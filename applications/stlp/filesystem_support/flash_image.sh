#!/bin/bash


echo "creating file system..."
if ./create_fs.sh ; then
    echo "flashing firmware and filesystem..."
    if xflash --quad-spi-clock 50MHz --factory ../bin/sw_avona.xe --boot-partition-size 0x100000 --data ./fat.fs ; then
        echo "xflash complete!"
    else
        echo "flash failed"
    fi
else
    echo "file system creation failed"
fi
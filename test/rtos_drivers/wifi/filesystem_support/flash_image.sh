if [ ! -f fat.fs ]; then
    ./create_fs.sh
fi

xflash --quad-spi-clock 50MHz --factory ../bin/rtos_drivers_wifi.xe --boot-partition-size 0x100000 --data ./fat.fs

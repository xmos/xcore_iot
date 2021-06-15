./create_fs.sh
xflash --quad-spi-clock 50MHz --factory ../bin/rtos_drivers_adv.xe --boot-partition-size 0x100000 --data ./fat.fs

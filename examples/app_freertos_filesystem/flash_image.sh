./create_fs.sh
xflash --factory bin/app_freertos_filesystem.xe --boot-partition-size 0x100000 --data ./fat.fs


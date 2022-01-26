@echo off

:: Create file system
echo "creating file system..."
call create_fs.bat

:: Flash firmware and filesystem
echo "flashing firmware and filesystem..."
xflash --quad-spi-clock 50MHz --factory ../bin/explorer_board.xe --boot-partition-size 0x100000 --data ./fat.fs
echo "xflash complete!"
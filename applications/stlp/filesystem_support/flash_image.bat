::*****************************************
:: This batch file flashes the Avona board.
::*****************************************
:: Notes * XTC Tools cmd prompt required.
::       * sw_avona firmware must be built.

@echo off


:: The sw_avona .xe resides here:
for /F "tokens=* USEBACKQ" %%F IN (`git rev-parse --show-toplevel`) DO (
set SW_AVONA_REPO_PATH=%%F
)
set AVONA_XE_PATH=%SW_AVONA_REPO_PATH%/bin

:: Create file system
echo "creating file system..."
call create_fs.bat

:: Flash firmware and filesystem
echo "flashing firmware and filesystem..."
xflash --quad-spi-clock 50MHz --factory %AVONA_XE_PATH%/sw_avona.xe --boot-partition-size 0x100000 --data ./fat.fs
echo "xflash complete!"

::*************************************************
:: This batch file creates an FAT filesystem.
::*************************************************

@echo off

:: Create directory for intended files and Copy renamed files into directory
if exist "%temp%\fatmktmp\" (
    :: Exit with error
    echo.
    echo fatmktmp\ directory already exists at %temp%
    echo Please delete and retry.
    pause
) else (
    mkdir %temp%\fs
    cp .\demo.txt %temp%\fs\demo.txt

    :: Run fatfs_mkimage.exe on the directory to create filesystem file
    start fatfs_mkimage.exe --input=%temp%\fs --output=fat.fs

    echo Filesystem created. Deleting temp files . . .
    :: File fat.fs is also deleted in cleanup without this:
    sleep 1

    :: Cleanup
    rm -rf %temp%\fatmktmp
)
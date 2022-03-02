::*************************************************
:: This batch file creates an Avona fat filesystem.
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
    mkdir %temp%\fatmktmp\ww\
    cp "%WW_PATH%\models\common\WR_250k.en-US.alexa.bin" %temp%\fatmktmp\ww\250kenUS.bin
    cp "%WW_PATH%\models\common\WS_50k.en-US.alexa.bin" %temp%\fatmktmp\ww\50kenUS.bin

    :: Run fatfs_mkimage.exe on the directory to create filesystem file
    start fatfs_mkimage.exe --input=%temp%\fatmktmp\ww --output=fat.fs

    echo Filesystem created. Deleting temp files . . .
    :: File fat.fs is also deleted in cleanup without this:
    sleep 1

    :: Cleanup
    rm -rf %temp%\fatmktmp
)

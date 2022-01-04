::***************************************************
:: This batch file creates the fatfs_mkimage utility.
::***************************************************

@echo off

setlocal enabledelayedexpansion

:: build program for fresh executables
set INSTALL_SCRIPT_PATH=%CD%
set HOSTAPP_PATH=%XCORE_SDK_PATH%/modules/rtos/sw_services/fatfs/host

cd %HOSTAPP_PATH%
rm -rf bin
rm -rf build
cmake -B build -G "NMake Makefiles"
cd build
nmake
cd %INSTALL_SCRIPT_PATH%

:: attain version of XCore SDK
:: read XCore SDK file "settings.json", removing line breaks
set jsonString=
for /f "delims=" %%x in (%XCORE_SDK_PATH%\settings.json) do set "jsonString=!jsonString!%%x"

:: remove braces
set "jsonString=%jsonString:~2,-2%"
:: remove quotes
set jsonString=%jsonString:"=%
:: change colon to equals
set "jsonString=%jsonString:: ==%"
:: parse into keys and values
set "%jsonString:, =" & set "%"

:: alert user where it is
echo.
echo.
echo Utility executables for fatfs_mkimage created at "%USERPROFILE%\.xmos\SDK\%version%\bin\".
echo Please add to the PATH system environment variable.
echo.
echo Then, test with fatfs_mkimage.exe --help
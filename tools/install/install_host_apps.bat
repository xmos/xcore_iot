::***************************************************
:: This batch file creates the fatfs_mkimage utility.
::***************************************************

@echo off

setlocal enabledelayedexpansion

:: install script is located in the current directory
set INSTALL_SCRIPT_PATH=%CD%
:: build files reside here:
FOR /F "tokens=* USEBACKQ" %%F IN (`git rev-parse --show-toplevel`) DO (
SET XCORE_SDK_REPO_PATH=%%F
)
set HOSTAPP_PATH=%XCORE_SDK_REPO_PATH%/modules/rtos/sw_services/fatfs/host

:: build program for new executables
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
echo Utility executables created at "%USERPROFILE%\.xmos\SDK\%version%\bin\".
echo Please add to the PATH system environment variable.
echo.
echo Then, test with fatfs_mkimage.exe --help
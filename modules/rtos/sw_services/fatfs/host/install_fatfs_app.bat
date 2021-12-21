::***************************************************
:: This batch file creates the fatfs_mkimage utility.
::***************************************************

@echo off

setlocal enabledelayedexpansion


:: build program for fresh executables
rm -rf bin
rm -rf build
cmake -B build -G "NMake Makefiles"
cd build
nmake

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

:: check if .xmos\SDK\%version%\bin directory exists
if exist "%USERPROFILE%\.xmos\SDK\%version%\bin" (
    echo.
    echo overwriting existing executables
) else (
    mkdir "%USERPROFILE%\.xmos\SDK\%version%\bin"
)

:: move fatfs_mkimage executables to %USERPROFILE%\.xmos\SDK\%version%\bin
cd ..
mv fat*.* "%USERPROFILE%"\.xmos\SDK\%version%\bin

:: create a convenient, session-only path environment variable to executables
set PATH=%PATH%;%USERPROFILE%\.xmos\SDK\%version%\bin

:: alert user where it is, and that it has been added to temporary env vars
echo.
echo.
echo Utility executables for fatfs_mkimage created at "%USERPROFILE%\.xmos\SDK\%version%\bin\".
echo Please add to the PATH system environment variable.
echo.
echo Test with fatfs_mkimage.exe --help
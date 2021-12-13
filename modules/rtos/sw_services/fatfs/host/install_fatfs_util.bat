::***************************************************
:: This batch file creates the fatfs_mkimage utility.
::***************************************************

@echo off


:: run CMake for fresh executables
rm -rf build
cmake -B build -G "NMake Makefiles"
cd build
nmake

:: check if .xutils\ directory exists
if exist "%USERPROFILE%\.xutils\" (
    echo.
    echo overwriting existing executables
) else (
    mkdir "%USERPROFILE%\.xutils\"
)

:: move fatfs_mkimage executables to %USERPROFILE%\.xutils
cd ..
mv fat*.* "%USERPROFILE%"\.xutils\

:: create a convenient, session-only path environment variable to executables
set PATH=%PATH%;%USERPROFILE%\.xutils

:: alert user where it is, and that it has been added to temporary env vars
echo.
echo Utility executables for fatfs_mkimage created at "%USERPROFILE%\.xutils\"
echo and PATH environment variable set for this instance only; add to environment
echo variables for permanence.
echo.
echo Test with fatfs_mkimage.exe --help
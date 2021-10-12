::***********************************************************
:: This batch file will remove the build and bin directories.
::***********************************************************

@echo off

rm -rf build
rm -rf bin

echo.
echo ***

set removed=
if exist build set removed=1
if exist bin set removed=1
if defined removed (
	echo Unable to remove directories.
) else (
	echo Removed build\ and bin\ directories.
)
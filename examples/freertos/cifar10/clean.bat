::***************************************************************
:: This batch file will call nmake clean on the build\ directory.
::***************************************************************

@echo off

if exist build (
	cd build
	if errorlevel==0 (
		nmake clean
		cd ..
	)
)
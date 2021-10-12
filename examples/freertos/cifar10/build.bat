::********************************************
:: This batch file will build the application.
::********************************************

@echo off

:: Preserve environment
setlocal

:: Get project name from directory
set dir_string=%~p0
for %%a in (%dir_string:\= %) do set project_name=%%a

:: The Explorer board is the default for this project:
set "default_board=XCORE-AI-EXPLORER"

echo.
echo ***

:: Check if board was supplied at cmd, otherwise use default
if [%1]==[] (
	echo Using default board: %default_board%.
	set "user_board=%default_board%"
) else (
	echo %1 has been chosen for the board.
	set "user_board=%1"
)

:: Provide confirmation and last chance to abort
echo Application will build for: %user_board%.
echo Please ctrl-C to abort.
echo.
pause

:: Build the makefile
cmake -G "NMake Makefiles" -B build -DBOARD=%user_board%

:: Check for build success
if exist "%~dp0\build\Makefile" (
	echo.
	echo Makefile build success.
	:: Navigate to the new build\ directory and run nmake on the Makefile.
	cd build
	nmake
) else (
	:: Exit with error
	echo.
	echo No Makefile found, aborting nmake.
	pause
)

if exist "%~dp0\bin\%project_name%.xe" (
	echo.
	echo ***
	echo Application built. Run with run.bat

endlocal
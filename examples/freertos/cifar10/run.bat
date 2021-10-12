::********************************************
:: This batch file will run the application.
::********************************************

@echo off

:: Preserve environment
setlocal

:: get project name from directory
set dir_string=%~p0
for %%a in (%dir_string:\= %) do set project_name=%%a

xrun --xscope bin\%project_name%.xe

endlocal
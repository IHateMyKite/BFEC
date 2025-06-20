@echo off

cd /D "%~dp0"

set preset="ALL"
if NOT "%1" == "" (
    set preset=%1
)

echo Running preset %preset%

cmake -S . --preset=%preset% 
if %ERRORLEVEL% NEQ 0 pause
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 pause

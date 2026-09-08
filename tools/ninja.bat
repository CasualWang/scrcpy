@echo off
if "%~1"=="--version" (
    echo 1.13.2
    exit /b 0
)
"D:\msys64\mingw64\bin\ninja.exe" %*

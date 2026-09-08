@echo off
setlocal

set "MSYS2=D:\msys64"
if not exist "%MSYS2%\usr\bin\bash.exe" (
    echo ERROR: MSYS2 not found at %MSYS2%
    echo Install MSYS2 to D:\msys64 or edit MSYS2 in this script.
    exit /b 1
)

set MSYSTEM=MINGW64
"%MSYS2%\usr\bin\bash.exe" --login -lc "export PATH=/mingw64/bin:/usr/bin:$PATH; cd /f/AndroidProject/scrcpy && ./build-scrcpy.sh"
exit /b %ERRORLEVEL%

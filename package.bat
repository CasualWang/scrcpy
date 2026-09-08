@echo off
setlocal
cd /d "%~dp0"

echo [1/2] Checking build artifacts...
if not exist "build\app\scrcpy.exe" (
    echo [ERROR] build\app\scrcpy.exe not found! Please run build-scrcpy.bat first.
    pause
    exit /b 1
)

if not exist "build\app\scrcpy-server" (
    echo [INFO] build\app\scrcpy-server not found, building server...
    call gradlew.bat assembleRelease
    copy /y server\build\outputs\apk\release\server-release-unsigned.apk build\app\scrcpy-server
)

echo [2/2] Packaging release bundle...
if exist "D:\msys64\mingw64\bin\python.exe" (
    "D:\msys64\mingw64\bin\python.exe" tools\package.py
) else (
    python tools\package.py
)

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================================
    echo [SUCCESS] Package created successfully in dist\
    echo ========================================================
) else (
    echo [ERROR] Packaging failed with code %ERRORLEVEL%
)

endlocal

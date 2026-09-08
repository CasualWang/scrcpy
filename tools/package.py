#!/usr/bin/env python3
import os
import sys
import shutil
import zipfile
import subprocess

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
BUILD_APP_DIR = os.path.join(PROJECT_ROOT, "build", "app")
SERVER_APK = os.path.join(PROJECT_ROOT, "server", "build", "outputs", "apk", "release", "server-release-unsigned.apk")
DIST_DIR = os.path.join(PROJECT_ROOT, "dist")
OUTPUT_FOLDER = os.path.join(DIST_DIR, "scrcpy-win64")
ZIP_OUTPUT = os.path.join(DIST_DIR, "scrcpy-win64-custom.zip")

OBJDUMP = r"D:\msys64\mingw64\bin\objdump.exe"
MINGW_BIN = r"D:\msys64\mingw64\bin"
ADB_DIR = r"F:\AndroidDevelop\Sdk\platform-tools"

SYSTEM_DLLS = {
    "kernel32.dll", "msvcrt.dll", "shell32.dll", "ws2_32.dll", "user32.dll",
    "gdi32.dll", "advapi32.dll", "ole32.dll", "oleaut32.dll", "imm32.dll",
    "version.dll", "winmm.dll", "setupapi.dll", "shlwapi.dll", "bcrypt.dll",
    "crypt32.dll", "secur32.dll", "iphlpapi.dll", "d3d9.dll", "d3d11.dll",
    "dxgi.dll", "opengl32.dll", "uxtheme.dll", "dwmapi.dll", "cfgmgr32.dll",
    "ntdll.dll", "comdlg32.dll", "winspool.drv"
}

def log(msg):
    print(f"[package] {msg}", flush=True)

def get_pe_dependencies(pe_file):
    cmd = [OBJDUMP, "-p", pe_file]
    try:
        res = subprocess.run(cmd, capture_output=True, text=True, errors="ignore")
    except Exception as e:
        log(f"Error running objdump on {pe_file}: {e}")
        return []
    deps = []
    for line in res.stdout.splitlines():
        if "DLL Name:" in line:
            dll = line.split("DLL Name:")[1].strip()
            deps.append(dll)
    return deps

def main():
    log(f"Project root: {PROJECT_ROOT}")
    scrcpy_exe = os.path.join(BUILD_APP_DIR, "scrcpy.exe")
    if not os.path.exists(scrcpy_exe):
        log("ERROR: build/app/scrcpy.exe not found! Please build the client first.")
        sys.exit(1)

    # 1. Clean and create output directory
    if os.path.exists(OUTPUT_FOLDER):
        shutil.rmtree(OUTPUT_FOLDER)
    os.makedirs(OUTPUT_FOLDER, exist_ok=True)
    log(f"Created release directory: {OUTPUT_FOLDER}")

    # 2. Copy scrcpy.exe
    shutil.copy2(scrcpy_exe, OUTPUT_FOLDER)
    log("Copied scrcpy.exe")

    # 3. Copy scrcpy-server
    scrcpy_server = os.path.join(BUILD_APP_DIR, "scrcpy-server")
    if not os.path.exists(scrcpy_server) and os.path.exists(SERVER_APK):
        scrcpy_server = SERVER_APK
    if os.path.exists(scrcpy_server):
        shutil.copy2(scrcpy_server, os.path.join(OUTPUT_FOLDER, "scrcpy-server"))
        log("Copied scrcpy-server")
    else:
        log("WARNING: scrcpy-server not found!")

    # 4. Copy navigation icons
    nav_src = os.path.join(PROJECT_ROOT, "app", "data", "nav")
    nav_dst = os.path.join(OUTPUT_FOLDER, "nav")
    os.makedirs(nav_dst, exist_ok=True)
    for f in os.listdir(nav_src):
        if f.endswith(".png"):
            shutil.copy2(os.path.join(nav_src, f), nav_dst)
    log("Copied nav icons to nav/")

    # 5. Copy extra assets & scripts
    app_data = os.path.join(PROJECT_ROOT, "app", "data")
    for extra in ["scrcpy-noconsole.vbs", "open_a_terminal_here.bat", "icon.ico", "scrcpy.png", "disconnected.png"]:
        p = os.path.join(app_data, extra)
        if os.path.exists(p):
            shutil.copy2(p, OUTPUT_FOLDER)

    # Add convenient console launcher
    console_bat = os.path.join(OUTPUT_FOLDER, "scrcpy-console.bat")
    with open(console_bat, "w", encoding="utf-8") as f:
        f.write("@echo off\nscrcpy.exe %*\nif %ERRORLEVEL% NEQ 0 pause\n")
    log("Created scrcpy-console.bat")

    license_file = os.path.join(PROJECT_ROOT, "LICENSE")
    if os.path.exists(license_file):
        shutil.copy2(license_file, os.path.join(OUTPUT_FOLDER, "LICENSE.txt"))

    # 6. Copy ADB tools
    for adb_file in ["adb.exe", "AdbWinApi.dll", "AdbWinUsbApi.dll"]:
        src_adb = os.path.join(ADB_DIR, adb_file)
        if os.path.exists(src_adb):
            shutil.copy2(src_adb, OUTPUT_FOLDER)
            log(f"Copied ADB tool: {adb_file}")
        else:
            log(f"WARNING: {src_adb} not found")

    # 7. Collect all MinGW DLL dependencies
    copied_dlls = set()
    to_scan = [scrcpy_exe]

    log("Analyzing and collecting DLL dependencies...")
    while to_scan:
        curr = to_scan.pop(0)
        for dll in get_pe_dependencies(curr):
            dll_lower = dll.lower()
            if dll_lower in SYSTEM_DLLS or dll_lower in copied_dlls:
                continue

            src = os.path.join(MINGW_BIN, dll)
            if os.path.exists(src):
                copied_dlls.add(dll_lower)
                dst = os.path.join(OUTPUT_FOLDER, dll)
                shutil.copy2(src, dst)
                to_scan.append(src)

    log(f"Successfully collected {len(copied_dlls)} runtime DLLs.")

    # 8. Create ZIP package
    if os.path.exists(ZIP_OUTPUT):
        os.remove(ZIP_OUTPUT)
    log(f"Creating ZIP archive: {ZIP_OUTPUT} ...")
    with zipfile.ZipFile(ZIP_OUTPUT, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(OUTPUT_FOLDER):
            for file in files:
                full_path = os.path.join(root, file)
                rel_path = os.path.relpath(full_path, DIST_DIR)
                zipf.write(full_path, rel_path)

    log(f"Release package generated successfully!")
    log(f"Folder: {OUTPUT_FOLDER}")
    log(f"Zip:    {ZIP_OUTPUT} ({os.path.getsize(ZIP_OUTPUT) / (1024*1024):.2f} MB)")

if __name__ == "__main__":
    main()

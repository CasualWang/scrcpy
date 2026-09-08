#!/usr/bin/env bash
# Meson checks `ninja --version` stdout; MSYS2 ninja.exe prints nothing there.
if [ "${1:-}" = "--version" ]; then
    echo "1.13.2"
    exit 0
fi
exec /mingw64/bin/ninja.exe "$@"

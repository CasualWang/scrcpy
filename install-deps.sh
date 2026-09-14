#!/usr/bin/env bash
set -euo pipefail

export PATH="/mingw64/bin:/usr/bin:$PATH"

echo "MSYS2 root: $(cygpath -w /)"
echo "Install target: D:\\msys64\\mingw64"
echo "Cache dir: D:\\msys64\\var\\cache\\pacman\\pkg"

pacman -Sy --noconfirm

pacman -S --needed --noconfirm \
    mingw-w64-x86_64-sdl3 \
    mingw-w64-x86_64-ffmpeg \
    mingw-w64-x86_64-libusb \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-pkgconf \
    mingw-w64-x86_64-meson \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-make \
    mingw-w64-x86_64-git

echo "Done. Installed tools:"
which gcc meson ninja pkg-config

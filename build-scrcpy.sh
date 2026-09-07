#!/usr/bin/env bash
set -euo pipefail

export PATH="/mingw64/bin:/usr/bin:$PATH"
export NINJA="/mingw64/bin/ninja.exe"

cd /f/AndroidProject/scrcpy

echo "=== tools check ==="
which gcc meson ninja pkg-config
ninja --version

NATIVE_FILE="--native-file=mingw64-native.ini"

echo "=== meson setup (client only) ==="
if [ ! -f build/build.ninja ]; then
    meson setup build $NATIVE_FILE --buildtype=release -Dcompile_server=false
else
    echo "build/ already configured, skipping meson setup"
    echo "(to reconfigure: rm -rf build && ./build-scrcpy.sh)"
fi

echo "=== ninja compile ==="
/mingw64/bin/ninja -C build

echo "=== copy nav icons (portable) ==="
mkdir -p build/app/nav
cp -f app/data/nav/*.png build/app/nav/

echo "=== build output ==="
ls -lh build/app/scrcpy.exe

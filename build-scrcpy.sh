#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export NINJA="${SCRIPT_DIR}/tools/ninja.bat"

cd "$SCRIPT_DIR"

echo "=== tools check ==="
which gcc meson ninja pkg-config
"$NINJA" --version

NATIVE_FILE="--native-file=mingw64-native.ini"

echo "=== meson setup (client only) ==="
if [ ! -f build/build.ninja ]; then
    meson setup build $NATIVE_FILE --buildtype=release -Dcompile_server=false
else
    echo "build/ already configured, skipping meson setup"
    echo "(to reconfigure: rm -rf build && ./build-scrcpy.sh)"
fi

echo "=== ninja compile ==="
"$NINJA" -C build

echo "=== copy nav icons (portable) ==="
mkdir -p build/app/nav
cp -f app/data/nav/*.png build/app/nav/

if [ -f server/build/outputs/apk/release/server-release-unsigned.apk ]; then
    echo "=== copy scrcpy-server (portable) ==="
    cp -f server/build/outputs/apk/release/server-release-unsigned.apk build/app/scrcpy-server
fi

echo "=== build output ==="
ls -lh build/app/scrcpy.exe
[ -f build/app/scrcpy-server ] && ls -lh build/app/scrcpy-server

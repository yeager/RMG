#!/usr/bin/env bash
set -ex

script_dir="$(dirname "$0")"
toplvl_dir="$(realpath "$script_dir/../../")"
bin_dir="$toplvl_dir/Bin/AppImage" # RMG should be installed here

export QMAKE="$(which qmake6)"
export EXTRA_PLATFORM_PLUGINS="libqwayland-generic.so"
export EXTRA_QT_PLUGINS="imageformats;iconengines;"
export VERSION="$(git describe --tags --always)"
export OUTPUT="$bin_dir/../RMG-Portable-Linux64-$VERSION.AppImage"
export LD_LIBRARY_PATH="$toplvl_dir/Build/AppImage/Source/RMG-Core" # hack

# hack because we cannot deploy integration plugins with linuxdeploy-qt,
# so copy it over manually
plugins_dir="$("$QMAKE" -query | grep QT_INSTALL_PLUGINS | cut -d':' -f2)"
mkdir -p "$bin_dir/usr/plugins/"
cp -r "$plugins_dir"/wayland-* "$bin_dir/usr/plugins/"

if [ ! -f "$script_dir/linuxdeploy-x86_64.AppImage" ]
then
    curl -L https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
        -o "$script_dir/linuxdeploy-x86_64.AppImage"
    chmod +x "$script_dir/linuxdeploy-x86_64.AppImage"
fi

if [ ! -f "$script_dir/linuxdeploy-plugin-qt-x86_64.AppImage" ]
then
    curl -L https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage \
        -o "$script_dir/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$script_dir/linuxdeploy-plugin-qt-x86_64.AppImage"
fi

"$script_dir/linuxdeploy-plugin-qt-x86_64.AppImage" --appimage-extract
"$script_dir/linuxdeploy-x86_64.AppImage" --appimage-extract

# delete appimages
rm "$script_dir/linuxdeploy-x86_64.AppImage" \
    "$script_dir/linuxdeploy-plugin-qt-x86_64.AppImage"

"$(pwd)/squashfs-root/AppRun" \
    --plugin=qt \
    --appdir="$bin_dir" \
    --custom-apprun="$script_dir/AppRun" \
    --output=appimage \
    --desktop-file="$bin_dir/usr/share/applications/com.github.Rosalie241.RMG.desktop"
#!/bin/bash
rm -rf _build
rm -rf _buildout
mkdir _build
mkdir _buildout
for name in linux-amd64 linux-armhf linux-i686 windows-x64 windows-x86; do
    mkdir _build/$name
    mkdir _buildout/$name
    meson --buildtype=release _build/$name --cross-file tools/cross-compilation/$name.txt
    ninja -C _build/$name
    cp _build/$name/qwprogs.* _buildout/$name/
done;



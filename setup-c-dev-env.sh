#!/usr/bin/env bash
# ==========================================================
# MSYS2 C Development Environment Setup Script
# Installs: clang, gtk3, curl, cjson, cmake, make, pkg-config
# Works in: MINGW64 shell
# ==========================================================

set -e  # Exit on any error

echo "🔧 Updating MSYS2 system packages..."
pacman -Syu --noconfirm

echo "📦 Installing base development tools..."
pacman -S --needed --noconfirm base-devel git

echo "🧠 Installing Clang and LLVM..."
pacman -S --needed --noconfirm mingw-w64-x86_64-clang mingw-w64-x86_64-llvm

echo "🌿 Installing GTK3 and pkg-config..."
pacman -S --needed --noconfirm mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config

echo "🌐 Installing libcurl..."
pacman -S --needed --noconfirm mingw-w64-x86_64-curl

echo "📦 Installing cJSON..."
pacman -S --needed --noconfirm mingw-w64-x86_64-cjson

echo "🛠️ Installing CMake and Make..."
pacman -S --needed --noconfirm mingw-w64-x86_64-cmake mingw-w64-x86_64-make

echo "✅ Installation complete!"
echo "-----------------------------------------------------------"
echo "Run the following command to verify your setup:"
echo 'clang `pkg-config --cflags gtk+-3.0` test.c -o test `pkg-config --libs gtk+-3.0` -lcurl -lcjson'
echo "-----------------------------------------------------------"
echo "🎉 All done! You can now build GTK, Curl, and cJSON apps with Clang."


# Android 构建环境：补足 vcpkg / git / 构建工具链
# 用法：nix-shell shell.nix
{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  packages = with pkgs; [
    git
    cmake
    ninja
    gnumake
    gcc
    autoconf
    automake
    libtool
    pkg-config
    vcpkg
    unzip
    zip
    curl
    jdk17
    gradle
    wget
    file
    perl
    python3
    which
    gawk
    xz
  ];

  # 保留系统已装的 Android SDK / NDK
  shellHook = ''
    export ANDROID_HOME="$ANDROID_HOME"
    export ANDROID_SDK_ROOT="$ANDROID_HOME"
    export ANDROID_API="24"
    export VCPKG_ROOT="$(vcpkg --version >/dev/null 2>&1; vcpkg root)"
  '';
}

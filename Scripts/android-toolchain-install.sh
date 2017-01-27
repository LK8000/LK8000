#!/bin/sh
#
# File:   android-toolchain-install.sh
# Author: bruno
#
# Created on Oct 17, 2016, 10:44:50 PM
#

ANDROID_SDK=$HOME/x-tools/android/android-sdk-linux
ANDROID_NDK=$HOME/x-tools/android/android-ndk-r13


SYSROOT=$ANDROID_NDK/platforms/android-19/arch-arm/
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --arch=arm --platform=android-19 --install-dir=$HOME/x-tools/android/standalone_toolchain/arm


SYSROOT=$ANDROID_NDK/platforms/android-19/arch-mips/
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --arch=mips --platform=android-19 --install-dir=$HOME/x-tools/android/standalone_toolchain/mips


SYSROOT=$ANDROID_NDK/platforms/android-19/arch-x86/
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --arch=x86 --platform=android-19 --install-dir=$HOME/x-tools/android/standalone_toolchain/x86


SYSROOT=$ANDROID_NDK/platforms/android-21/arch-x86_64/
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --arch=x86_64 --platform=android-21 --install-dir=$HOME/x-tools/android/standalone_toolchain/x86_64


SYSROOT=$ANDROID_NDK/platforms/android-21/arch-mips64/
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --arch=mips64 --platform=android-21 --install-dir=$HOME/x-tools/android/standalone_toolchain/mips64


SYSROOT=$ANDROID_NDK/platforms/android-21/arch-arm64/
$ANDROID_NDK/build/tools/make-standalone-toolchain.sh --arch=arm64 --platform=android-21 --install-dir=$HOME/x-tools/android/standalone_toolchain/arm64


chmod -R 755 $HOME/x-tools/android/standalone_toolchain/

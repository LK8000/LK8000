#!/bin/sh
# 
# File:   arm-tool-install.sh
# Author: Bruno de Lacheisserie
#
# Created on Jan 1, 2015, 6:39:30 PM
#

set -ex


[ -z "$TC"] && TC=arm-unknown-linux-gnueabi
[ -z "$BUILD_DIR" ] && BUILD_DIR=$HOME/tmp
[ -z "$TARGET_DIR" ] && TARGET_DIR=/opt/kobo/arm-unknown-linux-gnueabi

USER_PATH=$PATH

mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# install toolchain
#wget http://max.kellermann.name/download/xcsoar/devel/x-tools/x-tools-arm-i386-2013-12-11.tar.xz
#tar xJfC x-tools-arm-i386-2013-12-11.tar.xz /home/user
#export PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH

# install zlib ( 1.2.11 - 2017-01-15)
wget http://zlib.net/zlib-1.2.11.tar.gz
tar -xvzf zlib-1.2.11.tar.gz 
cd zlib-1.2.11
CC=$TC-gcc CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
./configure --prefix=$TARGET_DIR
make all
sudo make install
cd ..

# install zziplib ( 0.13.62 - 2012-03-11)
wget https://freefr.dl.sourceforge.net/project/zziplib/zziplib13/0.13.62/zziplib-0.13.62.tar.bz2
tar xvjf zziplib-0.13.62.tar.bz2 
mkdir zzipbuild
cd zzipbuild
CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
../zziplib-0.13.62/configure --host=$TC --target=$TC --prefix=$TARGET_DIR --with-zlib
make all
sudo PATH=$USER_PATH:$PATH \
    make install
cd ..

# install boostlib ( 1.66.0 - 2017-12-18 )
wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
tar xzf boost_1_66_0.tar.gz
cd boost_1_66_0
./bootstrap.sh
echo "using gcc : arm : $TC-g++ : cxxflags=-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations ;" > user-config.jam
sudo PATH=$USER_PATH:$PATH \
    ./bjam toolset=gcc \
           variant=release \
           link=shared \
           runtime-link=shared \
           --prefix=$TARGET_DIR \
           --without-python \
           --without-context \
           --without-coroutine \
           --user-config=user-config.jam \
           install
cd ..

# install libpng ( 1.6.34 - 2017-09-29 )
wget http://sourceforge.net/projects/libpng/files/libpng16/1.6.34/libpng-1.6.34.tar.gz
tar xzf libpng-1.6.34.tar.gz
mkdir libpng-build
cd libpng-build
../libpng-1.6.34/configure \
    --host=$TC \
    CC=$TC-gcc \
    AR=$TC-ar \
    STRIP=$TC-strip \
    RANLIB=$TC-ranlib \
    CPPFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations -I$TARGET_DIR/include" \
    LDFLAGS="-L$TARGET_DIR/lib" \
    --prefix=$TARGET_DIR \
    --enable-arm-neon
make
sudo PATH=$USER_PATH:$PATH \
    make install
cd ..

# install freetype2 ( 2.9 - 2018-01-08 )
wget https://download.savannah.gnu.org/releases/freetype/freetype-2.9.tar.gz
tar xzf freetype-2.9.tar.gz
mkdir freetype-build
cd freetype-build
CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
LDFLAGS="-L$TARGET_DIR/lib"  \
../freetype-2.9/configure \
    --host=$TC \
    --target=$TC \
    --prefix=$TARGET_DIR \
    --without-harfbuzz \
    PKG_CONFIG_LIBDIR=$TARGET_DIR/lib/pkgconfig
make
sudo PATH=$USER_PATH:$PATH \
    make install
cd ..

# install Geographiclib ( 1.49 - 2017-10-05 )
wget https://netcologne.dl.sourceforge.net/project/geographiclib/distrib/GeographicLib-1.49.tar.gz
tar xzf GeographicLib-1.49.tar.gz
mkdir GeographicLib-build
cd GeographicLib-build
CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
LDFLAGS="-L$TARGET_DIR/lib"  \
../GeographicLib-1.49/configure \
    --host=$TC \
    --prefix=$TARGET_DIR \
    PKG_CONFIG_LIBDIR=$TARGET_DIR/lib/pkgconfig
make
sudo PATH=$USER_PATH:$PATH \
    make install
cd ..

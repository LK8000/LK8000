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

if [ command -v ${TC}-gcc >/dev/null ]; then
    echo "error : ${TC} toolchain not available"
    exit 1
fi

NB_CORES=$(grep -c '^processor' /proc/cpuinfo)
export MAKEFLAGS="-j$((NB_CORES+1)) -l${NB_CORES}"

[ ! -d ${BUILD_DIR} ] && mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}


# install zlib ( 1.2.11 - 2017-01-15)
[ ! -f zlib-1.2.11.tar.gz ] && wget http://zlib.net/zlib-1.2.11.tar.gz
[ -d zlib-1.2.11 ] && rm -rf zlib-1.2.11
tar -xvzf zlib-1.2.11.tar.gz
cd zlib-1.2.11
CC=$TC-gcc CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
./configure --prefix=$TARGET_DIR
make all && make install
cd ..

# install zziplib ( 0.13.69 - 2018-03-17)
[ ! -f v0.13.69.tar.gz ] && wget https://github.com/gdraheim/zziplib/archive/v0.13.69.tar.gz
[ -d zziplib-0.13.69 ] && rm -rf zziplib-0.13.69
[ -d zzipbuild ] && rm -rf zzipbuild
tar -xvzf v0.13.69.tar.gz 
mkdir zzipbuild
cd zzipbuild
CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
../zziplib-0.13.69/configure --host=$TC --target=$TC --prefix=$TARGET_DIR --with-zlib
make all && make install
cd ..

# install boostlib ( 1.69.0 - 2018-12-12 )
[ ! -f boost_1_69_0.tar.gz ] && wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz
[ -d boost_1_69_0 ] && rm -rf boost_1_69_0
tar xzf boost_1_69_0.tar.gz
cd boost_1_69_0
./bootstrap.sh
echo "using gcc : arm : $TC-g++ : cxxflags=-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations ;" > user-config.jam
./bjam toolset=gcc-arm \
           variant=release \
           link=shared \
           runtime-link=shared \
           --prefix=$TARGET_DIR \
           --without-python \
           --without-context \
           --without-coroutine \
           --without-fiber \
           --address-model=32 \
           --without-mpi \
           --without-graph_parallel \
           -sZLIB_SOURCE="$BUILD_DIR/zlib-1.2.11" \
           -sZLIB_INCLUDE="$TARGET_DIR\include" \
           -sZLIB_LIBPATH="$TARGET_DIR\lib" \
           --user-config=user-config.jam \
           install
cd ..

# install libpng ( 1.6.37 - 2019-04-15 )
[ ! -f libpng-1.6.37.tar.gz ] && wget http://sourceforge.net/projects/libpng/files/libpng16/1.6.37/libpng-1.6.37.tar.gz
[ -d libpng-1.6.37 ] && rm -rf libpng-1.6.37
[ -d libpng-build ] && rm -rf libpng-build
tar xzf libpng-1.6.37.tar.gz
mkdir libpng-build
cd libpng-build
../libpng-1.6.37/configure \
    --host=$TC \
    CC=$TC-gcc \
    AR=$TC-ar \
    STRIP=$TC-strip \
    RANLIB=$TC-ranlib \
    CPPFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations -I$TARGET_DIR/include" \
    LDFLAGS="-L$TARGET_DIR/lib" \
    --prefix=$TARGET_DIR \
    --enable-arm-neon
make && make install
cd ..

# install freetype2 ( 2.10.1 - 2019-07-01 )
[ ! -f freetype-2.10.1.tar.gz ] && wget https://download.savannah.gnu.org/releases/freetype/freetype-2.10.1.tar.gz
[ -d freetype-2.10.1 ] && rm -rf freetype-2.10.1
[ -d freetype-build ] && rm -rf freetype-build
tar xzf freetype-2.10.1.tar.gz
mkdir freetype-build
cd freetype-build
CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
LDFLAGS="-L$TARGET_DIR/lib"  \
../freetype-2.10.1/configure \
    --host=$TC \
    --target=$TC \
    --prefix=$TARGET_DIR \
    --without-harfbuzz \
    PKG_CONFIG_LIBDIR=$TARGET_DIR/lib/pkgconfig
make && make install
cd ..

# install Geographiclib ( 1.50.1 -  2019-12-13 )
[ ! -f GeographicLib-1.50.1.tar.gz ] && wget https://netcologne.dl.sourceforge.net/project/geographiclib/distrib/GeographicLib-1.49.tar.gz
[ -d GeographicLib-1.50.1 ] && rm -rf GeographicLib-1.50.1
[ -d GeographicLib-build ] && rm -rf GeographicLib-build
tar xzf GeographicLib-1.50.1.tar.gz
mkdir GeographicLib-build
cd GeographicLib-build
CFLAGS="-O3 -march=armv7-a -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -funsafe-math-optimizations -funsafe-loop-optimizations" \
LDFLAGS="-L$TARGET_DIR/lib"  \
../GeographicLib-1.50.1/configure \
    --host=$TC \
    --prefix=$TARGET_DIR \
    PKG_CONFIG_LIBDIR=$TARGET_DIR/lib/pkgconfig
make && make install
cd ..

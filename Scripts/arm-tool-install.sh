#!/bin/sh
# 
# File:   arm-tool-install.sh
# Author: Bruno de Lacheisserie
#
# Created on Jan 1, 2015, 6:39:30 PM
#

mkdir /lk/tmp
cd /lk/tmp

# install toolchain
wget http://max.kellermann.name/download/xcsoar/devel/x-tools/x-tools-arm-i386-2013-12-11.tar.xz
tar xJfC x-tools-arm-i386-2013-12-11.tar.xz /home/user
export PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH

# install zlib ( 1.2.8 - 2013-04-28)
wget http://zlib.net/zlib-1.2.8.tar.gz
tar -xvzf zlib-1.2.8.tar.gz 
cd zlib-1.2.8
CC=arm-unknown-linux-gnueabi-gcc CFLAGS="-O3 -march=armv7-a -mfpu=neon -mfloat-abi=hard -ftree-vectorize" \
./configure --prefix=/opt/kobo/arm-unknown-linux-gnueabi
make all
sudo make install
cd ..

# install zziplib ( 0.13.62 - 2012-03-11)
wget http://liquidtelecom.dl.sourceforge.net/project/zziplib/zziplib13/0.13.62/zziplib-0.13.62.tar.bz2
tar xvjf zziplib-0.13.62.tar.bz2 
mkdir zzipbuild
cd zzipbuild
CFLAGS="-O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard -ftree-vectorize" \
../zziplib-0.13.62/configure --host=arm-unknown-linux-gnueabi --target=arm-unknown-linux-gnueabi --prefix=/opt/kobo/arm-unknown-linux-gnueabi --with-zlib
make all
sudo PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH \
    make install
cd ..

# install boostlib ( 1.57.0 - 2014-10-03 )
wget http://netcologne.dl.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.tar.gz
tar xzf boost_1_57_0.tar.gz
cd boost_1_57_0
./bootstrap.sh
echo "using gcc : arm : arm-unknown-linux-gnueabi-g++ : cxxflags=-march=armv7-a -mfpu=neon -mfloat-abi=hard -ftree-vectorize ;" > user-config.jam
sudo PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH \
    bjam toolset=gcc target-os=linux variant=release link=shared runtime-link=shared --prefix=/opt/kobo/arm-unknown-linux-gnueabi --user-config=user-config.jam install
cd ..

# install libpng ( 1.6.16 - 2014-12-22 )
wget http://switch.dl.sourceforge.net/project/libpng/libpng16/1.6.16/libpng-1.6.16.tar.gz
tar xzf libpng-1.6.16.tar.gz
mkdir libpng-build
cd libpng-build
../libpng-1.6.16/configure \
    --host=arm-unknown-linux-gnueabi \
    CC=arm-unknown-linux-gnueabi-gcc \
    AR=arm-unknown-linux-gnueabi-ar \
    STRIP=arm-unknown-linux-gnueabi-strip \
    RANLIB=arm-unknown-linux-gnueabi-ranlib \
    CPPFLAGS="-O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard -ftree-vectorize -I/opt/kobo/arm-unknown-linux-gnueabi/include" \
    LDFLAGS="-L/opt/kobo/arm-unknown-linux-gnueabi/lib"  \
    --prefix=/opt/kobo/arm-unknown-linux-gnueabi 
make
sudo PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH \
    make install
cd ..

# install freetype2 ( 2.5.5 - 2014-12-30 )
wget http://download.savannah.gnu.org/releases/freetype/freetype-2.5.5.tar.gz
tar xzf freetype-2.5.5.tar.gz
mkdir freetype-build
cd freetype-build
CFLAGS="-O2 -march=armv7-a -mfpu=neon -mfloat-abi=hard -ftree-vectorize" \
../freetype-2.5.5/configure \
    --host=arm-unknown-linux-gnueabi \
    --target=arm-unknown-linux-gnueabi \
    --prefix=/opt/kobo/arm-unknown-linux-gnueabi \
    PKG_CONFIG_LIBDIR=/opt/kobo/arm-unknown-linux-gnueabi/lib/pkgconfig
make
sudo PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH \
    make install
cd ..

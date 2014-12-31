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

#install library
wget http://max.kellermann.name/download/xcsoar/devel/kobo/kobo-libs-2013-12-11.tar.xz
sudo tar xJfC kobo-libs-2013-12-11.tar.xz /opt

#install zlib
wget http://zlib.net/zlib-1.2.8.tar.gz
tar -xvzf zlib-1.2.8.tar.gz 
cd zlib-1.2.8
CC=arm-unknown-linux-gnueabi-gcc \
./configure --prefix=/opt/kobo/arm-unknown-linux-gnueabi
make 
sudo make install
cd ..

#install zziplib
wget http://liquidtelecom.dl.sourceforge.net/project/zziplib/zziplib13/0.13.62/zziplib-0.13.62.tar.bz2
tar xvjf zziplib-0.13.62.tar.bz2 
mkdir zzipbuild
cd zzipbuild
../zziplib-0.13.62/configure --host=arm-unknown-linux-gnueabi --target=arm-unknown-linux-gnueabi --prefix=/opt/kobo/arm-unknown-linux-gnueabi --with-zlib
make
sudo PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH \
make install
cd ..

#install boostlib
wget http://netcologne.dl.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.tar.gz
tar xzf boost_1_57_0.tar.gz
cd boost_1_57_0
./bootstrap.sh
echo "using gcc : arm : arm-unknown-linux-gnueabi-g++ ;" > user-config.jam
sudo PATH=/home/user/x-tools/arm-unknown-linux-gnueabi/bin:$PATH \
bjam toolset=gcc target-os=linux variant=release link=shared runtime-link=shared --prefix=/opt/kobo/arm-unknown-linux-gnueabi --user-config=user-config.jam install
cd ..

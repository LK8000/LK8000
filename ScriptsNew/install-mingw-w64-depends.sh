#!/bin/sh
# 
# File:   install-mingw-w64-depends.sh
# Author: Bruno de Lacheisserie
#
# Created on Dec 14, 2016, 10:48:50 PM
#

NB_CORES=$(grep -c '^processor' /proc/cpuinfo)
export MAKEFLAGS="-j$((NB_CORES+1)) -l${NB_CORES}"

DOWNLOAD_DIR="${HOME}/tmp/download/"
SOURCE_DIR="${HOME}/tmp/source/"
BUILD_DIR="${HOME}/tmp/build/"

TOOLCHAINS="i686-w64-mingw32 x86_64-w64-mingw32"

# this is valid for debian
PREFIX_DIR="/usr"

GEOGRAPHICLIB_VER="1.50.1"


# install additional library dependencies for PC target

if [ ! -f ${DOWNLOAD_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz ]; then

    [ ! -d ${DOWNLOAD_DIR} ] && mkdir -p ${DOWNLOAD_DIR}
    cd ${DOWNLOAD_DIR}
    wget http://freefr.dl.sourceforge.net/project/geographiclib/distrib/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz
    [ -d ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER} ] && rm -rf ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}

fi

if [ ! -d ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER} ]; then
    [ ! -d ${SOURCE_DIR} ] && mkdir -p ${SOURCE_DIR}
    cd ${SOURCE_DIR}
    tar xzf ${DOWNLOAD_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz

    for TC in TOOLCHAINS; do
        [ -d ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER} ] && rm -rf ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER}
    done

fi

for TC in ${TOOLCHAINS}; do

    # if not able to run TC  not exist ignore this toolchain
   ! command -v ${TC}-gcc >/dev/null && continue

    # if Prefix directory not exist ignore this toolchain
    [ ! -d ${PREFIX_DIR}/${TC} ] && continue


    [ ! -d ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER} ] && mkdir -p ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER}

    cd ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER}

    ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}/configure \
            --host=${TC} \
            --prefix=${PREFIX_DIR}/${TC} \
            PKG_CONFIG_LIBDIR=${PREFIX_DIR}/${TC}/lib/pkgconfig

    make && make install

done

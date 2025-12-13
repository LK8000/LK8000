#!/bin/bash
# 
# File:   install-mingw-w64-depends.sh
# Author: Bruno de Lacheisserie
#
# Created on Dec 14, 2016, 10:48:50 PM
#

SCRIPT_DIR=$(realpath "`dirname $0`")

NB_CORES=$(grep -c '^processor' /proc/cpuinfo)
export MAKEFLAGS="-j$((NB_CORES+1)) -l${NB_CORES}"

DOWNLOAD_DIR="${HOME}/tmp/download/"
SOURCE_DIR="${HOME}/tmp/source/"
BUILD_DIR="${HOME}/tmp/build/"

TOOLCHAINS="i686-w64-mingw32 x86_64-w64-mingw32"

# this is valid for debian
PREFIX_DIR="/usr"

GEOGRAPHICLIB_VER="2.5.1"

[ ! -d ${DOWNLOAD_DIR} ] && mkdir -p ${DOWNLOAD_DIR}
[ ! -d ${SOURCE_DIR} ] && mkdir -p ${SOURCE_DIR}
[ ! -d ${BUILD_DIR} ] && mkdir -p ${BUILD_DIR}

# install additional library dependencies for PC target

pushd ${DOWNLOAD_DIR}

if [ ! -f ${DOWNLOAD_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz ]; then
    wget http://freefr.dl.sourceforge.net/project/geographiclib/distrib-C%2B%2B/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz
    [ -d ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER} ] && rm -rf ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}
fi

if [ ! -f ${DOWNLOAD_DIR}/master.tar.gz ]; then
    wget https://github.com/brunotl/zziplib/archive/refs/heads/master.tar.gz
    [ -d "${SOURCE_DIR}/zziplib-master" ] && rm -rf "${SOURCE_DIR}/zziplib-master"
fi

popd

pushd ${SOURCE_DIR}

if [ ! -d ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER} ]; then
    tar xzf ${DOWNLOAD_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz

    for TC in TOOLCHAINS; do
        [ -d ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER} ] && rm -rf ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER}
    done

fi

if [ ! -d ${SOURCE_DIR}/zziplib-master ]; then
    tar xzf ${DOWNLOAD_DIR}/master.tar.gz
    for TC in TOOLCHAINS; do
        [ -d ${BUILD_DIR}/${TC}/zziplib-master ] && rm -rf ${BUILD_DIR}/${TC}/zziplib-master
    done
fi

popd

for TC in ${TOOLCHAINS}; do

    # if not able to run TC  not exist ignore this toolchain
   ! command -v ${TC}-gcc >/dev/null && continue

    # if Prefix directory not exist ignore this toolchain
    [ ! -d ${PREFIX_DIR}/${TC} ] && continue

    [ ! -d ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER} ] && mkdir -p ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER}

    pushd ${BUILD_DIR}/${TC}/GeographicLib-${GEOGRAPHICLIB_VER}

    ${SOURCE_DIR}/GeographicLib-${GEOGRAPHICLIB_VER}/configure \
            --host=${TC} \
            --prefix=${PREFIX_DIR}/${TC} \
            PKG_CONFIG_LIBDIR=${PREFIX_DIR}/${TC}/lib/pkgconfig

    make && make install

    popd

    [ ! -d ${BUILD_DIR}/${TC}/zziplib-master ] && mkdir -p ${BUILD_DIR}/${TC}/zziplib-master

    pushd ${BUILD_DIR}/${TC}/zziplib-master

        cmake ${SOURCE_DIR}/zziplib-master \
           -DCMAKE_TOOLCHAIN_FILE=${SCRIPT_DIR}/${TC}.cmake \
            -DCMAKE_INSTALL_PREFIX=${PREFIX_DIR}/${TC} \
            -DBUILD_SHARED_LIBS=OFF \
            -DBUILD_TESTS=OFF \
            -DMSVC_STATIC_RUNTIME=OFF \
            -DZZIPMMAPPED=ON \
            -DZZIPFSEEKO=OFF \
            -DZZIPWRAP=OFF \
            -DZZIPSDL=OFF \
            -DZZIPBINS=OFF \
            -DZZIPTEST=OFF \
            -DZZIPDOCS=OFF \
            -DFORTIFY=OFF \

        make install

    popd

done

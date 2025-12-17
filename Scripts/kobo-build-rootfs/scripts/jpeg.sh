#!/bin/bash
set -e -u


ARCHIVE_URL=https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.1.3/libjpeg-turbo-3.1.3.tar.gz
ARCHIVE=libjpeg-turbo-3.1.3.tar.gz
ARCHIVEDIR=libjpeg-turbo-3.1.3

. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

    SYSROOT=$(${CROSSTARGET}-gcc --print-sysroot) \
    CFLAGS="${CFLAGS} -funwind-tables" \
	CPPFLAGS=${CPPFLAGS} \
    CROSSTARGET=${CROSSTARGET} \
    DEVICEROOT=${DEVICEROOT} \
    PKG_CONFIG_PATH=:$DEVICEROOT/lib/pkgconfig \
    cmake \
        -DCMAKE_TOOLCHAIN_FILE=${KOBO_SCRIPT_DIR}/arm-kobo-linux-gnueabihf.cmake \
        -DCMAKE_INSTALL_PREFIX=${DEVICEROOT} \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DBUILD_SHARED_LIBS=ON \
        -DENABLE_STATIC=ON \
        -DENABLE_SHARED=OFF \
        

	$MAKE -j$MAKE_JOBS -f Makefile install

popd
markbuilt

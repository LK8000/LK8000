#!/bin/bash
set -e -u

ARCHIVE_URL=https://github.com/gdraheim/zziplib/archive/refs/tags/v0.13.80.tar.gz
ARCHIVE=v0.13.80.tar.gz
ARCHIVEDIR=zziplib-0.13.80

. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

    SYSROOT=$(${CROSSTARGET}-gcc --print-sysroot) \
    CFLAGS=${CFLAGS} \
	CPPFLAGS=${CPPFLAGS} \
    CROSSTARGET=${CROSSTARGET} \
    DEVICEROOT=${DEVICEROOT} \
    PKG_CONFIG_PATH=:$DEVICEROOT/lib/pkgconfig \
    cmake \
        -DCMAKE_TOOLCHAIN_FILE=${KOBO_SCRIPT_DIR}/arm-kobo-linux-gnueabihf.cmake \
        -DCMAKE_INSTALL_PREFIX=${DEVICEROOT} \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DBUILD_SHARED_LIBS=ON \
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


	$MAKE -j$MAKE_JOBS -f Makefile install

popd
markbuilt

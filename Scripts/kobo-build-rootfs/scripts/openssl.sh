#!/bin/bash
set -e -u

ARCHIVE_URL=https://www.openssl.org/source/openssl-3.2.1.tar.gz
ARCHIVE=openssl-3.2.1.tar.gz
ARCHIVEDIR=openssl-3.2.1
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

    PKG_CONFIG_PATH="${DEVICEROOT}/lib/pkgconfig" \
    CFLAGS="${CFLAGS}" \
    LDFLAGS="${LDFLAGS} -latomic"  \
    CPPFLAGS="${CPPFLAGS}" \
    ./Configure \
        linux-generic32 \
        --cross-compile-prefix=${CROSSTARGET}- \
        --prefix=${DEVICEROOT} \
        no-asm no-module no-engine no-static-engine no-async \
        no-tests no-docs zlib

	$MAKE -j$MAKE_JOBS
	$MAKE install

popd
markbuilt

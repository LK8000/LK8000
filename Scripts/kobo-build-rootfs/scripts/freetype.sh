#!/bin/bash
set -e -u

ARCHIVE_URL=https://sourceforge.net/projects/freetype/files/freetype2/2.13.2/freetype-2.13.2.tar.gz
ARCHIVE=freetype-2.13.2.tar.gz
ARCHIVEDIR=freetype-2.13.2
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

    CFLAGS="${CFLAGS}" \
    LDFLAGS="${LDFLAGS}"  \
    ./configure \
        --host=$CROSSTARGET \
        --target=$CROSSTARGET \
        --prefix=$DEVICEROOT \
        --without-harfbuzz \
        PKG_CONFIG_LIBDIR=$DEVICEROOT/lib/pkgconfig

	$MAKE -j$MAKE_JOBS
	$MAKE install
popd
markbuilt
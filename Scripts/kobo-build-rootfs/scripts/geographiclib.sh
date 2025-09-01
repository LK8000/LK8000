#!/bin/bash
set -e -u

GEOGRAPHICLIB_VER="2.5.1"

ARCHIVE_URL=https://netcologne.dl.sourceforge.net/project/geographiclib/distrib-C%2B%2B/GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz
ARCHIVE=GeographicLib-${GEOGRAPHICLIB_VER}.tar.gz
ARCHIVEDIR=GeographicLib-${GEOGRAPHICLIB_VER}
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

    CFLAGS="${CFLAGS}" \
    LDFLAGS="${LDFLAGS}"  \
    ./configure \
        --host=$CROSSTARGET \
        --prefix=$DEVICEROOT \
        PKG_CONFIG_LIBDIR=$DEVICEROOT/lib/pkgconfig

	$MAKE -j$MAKE_JOBS
	$MAKE install

popd
markbuilt
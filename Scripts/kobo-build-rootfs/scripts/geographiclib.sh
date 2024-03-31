#!/bin/bash
set -e -u

ARCHIVE_URL=https://netcologne.dl.sourceforge.net/project/geographiclib/distrib-C%2B%2B/GeographicLib-2.3.tar.gz
ARCHIVE=GeographicLib-2.3.tar.gz
ARCHIVEDIR=GeographicLib-2.3
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
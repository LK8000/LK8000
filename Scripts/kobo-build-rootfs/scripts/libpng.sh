#!/bin/bash
set -e -u

ARCHIVE_URL=http://sourceforge.net/projects/libpng/files/libpng16/1.6.43/libpng-1.6.43.tar.gz
ARCHIVE=libpng-1.6.43.tar.gz
ARCHIVEDIR=libpng-1.6.43
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

	CPPFLAGS="${CPPFLAGS}" \
	LDFLAGS="${LDFLAGS}" \
	./configure \
		--with-libpng-compat \
		--prefix=${DEVICEROOT} \
		--host=${CROSSTARGET} \
		--enable-arm-neon
	
	$MAKE -j$MAKE_JOBS
	$MAKE install
popd
markbuilt

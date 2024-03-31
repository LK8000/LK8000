#!/bin/bash
set -e -u

ARCHIVE_URL=https://zlib.net/fossils/zlib-1.2.13.tar.gz
ARCHIVE=zlib-1.2.13.tar.gz
ARCHIVEDIR=zlib-1.2.13
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

	LD=${CROSSTARGET}-ld \
	CC=${CC} \
	./configure \
		--prefix=${DEVICEROOT}

	$MAKE -j$MAKE_JOBS
	$MAKE install

popd
markbuilt

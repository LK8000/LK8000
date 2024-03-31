#!/bin/bash
set -e -u

ARCHIVE_URL=https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.gz
ARCHIVE=boost_1_84_0.tar.gz
ARCHIVEDIR=boost_1_84_0
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

    ./bootstrap.sh
    echo "using gcc : arm : $CROSSTARGET-g++ : cxxflags=$CPPFLAGS ;" > user-config.jam
    ./b2 toolset=gcc-arm \
            -q -d1 \
            variant=release \
            link=shared \
            runtime-link=shared \
            --prefix=$DEVICEROOT \
            --address-model=32 \
            --with-headers \
            -sZLIB_INCLUDE="$DEVICEROOT\include" \
            -sZLIB_LIBPATH="$DEVICEROOT\lib" \
            --user-config=user-config.jam \
            install

popd
markbuilt
#!/bin/bash
set -e -u

ARCHIVE_URL=https://github.com/curl/curl/releases/download/curl-8_7_1/curl-8.7.1.tar.gz
ARCHIVE=curl-8.7.1.tar.gz
ARCHIVEDIR=curl-8.7.1
. $KOBO_SCRIPT_DIR/build-common.sh

pushd $ARCHIVEDIR

	PKG_CONFIG_PATH="${DEVICEROOT}/lib/pkgconfig" \
    CFLAGS="${CFLAGS}" \
    LDFLAGS="${LDFLAGS}"  \
    CPPFLAGS="${CPPFLAGS}" \
    ./configure \
		--host=${CROSSTARGET} \
		--prefix=${DEVICEROOT} \
        --disable-static \
        --enable-http --enable-https \
        --enable-ipv6 \
        --enable-ftp --disable-file \
        --disable-ldap --disable-ldaps \
        --disable-rtsp --disable-proxy --disable-dict --disable-telnet \
        --disable-tftp --disable-pop3 --disable-imap --disable-smb \
        --disable-smtp --disable-gopher --disable-manual \
        --disable-threaded-resolver --disable-sspi \
        --disable-crypto-auth --disable-ntlm-wb --disable-tls-srp --disable-cookies \
        --with-openssl --without-gnutls --without-nss --without-libssh2 \
        --disable-docs

#        --disable-shared --enable-static \
#        --disable-debug \

	$MAKE -j$MAKE_JOBS
	$MAKE install

popd
markbuilt

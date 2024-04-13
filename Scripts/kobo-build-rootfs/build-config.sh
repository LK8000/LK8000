# Read configuration overrides file from the build directory, if present. Use
# this file instead of editing build-config.sh directly, as it will never be
# checked into revision control.
#
# All these variables may be overridden by setting them in build-config-user.sh
# or in the environment, as values are only set if the variable is empty.
#
[ -e $KOBO_SCRIPT_DIR/build-config-user.sh ] && . $KOBO_SCRIPT_DIR/build-config-user.sh

# Where produced binaries should be installed to. It must not have a trailing slash.
def_device_root=/opt/kobo-rootfs
DEVICEROOT=${DEVICEROOT:-$def_device_root}

def_archives_dir=$KOBO_SCRIPT_DIR/../packages
def_patches_dir=$KOBO_SCRIPT_DIR/../patches
ARCHIVESDIR=${ARCHIVESDIR:-$def_archives_dir}
PATCHESDIR=${PATCHESDIR:-$def_patches_dir}

# How to invoke make
MAKE=${MAKE:-"make"}

def_make_jobs=$(grep -c '^processor' /proc/cpuinfo)
MAKE_JOBS=${MAKE_JOBS:-$def_make_jobs}

# Target host. Omit any trailing hyphen.
CROSSTARGET=${CROSSTARGET:-"arm-kobo-linux-gnueabihf"}

# Invocation of tools. These should NOT be exported to the environment; the
# individual scripts decide how and when to pass them to commands.
CPPFLAGS=${CPPFLAGS:-"-I${DEVICEROOT}/include"}
CFLAGS=${CFLAGS:-"${CPPFLAGS}"}
LIBS=${LIBS:-"-L${DEVICEROOT}/lib"}
LDFLAGS=${LDFLAGS:-"${LIBS}"}

LIBTOOL=${LIBTOOL:-"${DEVICEROOT}/usr/local/bin/${CROSSTARGET}-libtool"}
CC=${CC:-"${CROSSTARGET}-gcc"}

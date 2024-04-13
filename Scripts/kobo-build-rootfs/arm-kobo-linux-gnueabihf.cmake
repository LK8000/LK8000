set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER $ENV{CROSSTARGET}-gcc)
set(CMAKE_CXX_COMPILER $ENV{CROSSTARGET}-g++)

set(CMAKE_FIND_ROOT_PATH  
    $ENV{SYSROOT}
    $ENV{DEVICEROOT}
)

set(CPACK_PACKAGE_ARCHITECTURE, armhf)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE, armhf)
set(PACK_RPM_PACKAGE_ARCHITECTURE, armv7hl)

set(CMAKE_SYSROOT $ENV{SYSROOT})

# Retrieve the machine supported by the toolchain
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE TOOLCHAIN_MACHINE OUTPUT_STRIP_TRAILING_WHITESPACE)

# Add '--sysroot' to the compiler flags
set(ENV{CFLAGS} "--sysroot=$ENV{SYSROOT} $ENV{CFLAGS}")

# Add '-rpath' to the linker flags
set(ENV{LDFLAGS} "--sysroot=$ENV{SYSROOT} -Wl,-rpath,$ENV{SYSROOT}/lib/${TOOLCHAIN_MACHINE} $ENV{LDFLAGS}")

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS $ENV{CFLAGS})
set(CMAKE_CXX_FLAGS $ENV{CPPFLAGS})

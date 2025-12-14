
cmake_minimum_required(VERSION 3.10)
project(cupx_reader CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver zziplib
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZZIP REQUIRED zziplib)
pkg_check_modules(ZZIPMAP REQUIRED zzipmmapped)

# Ajouter l'exécutable
add_executable(cupx_reader
    main.cpp
    ../../Source/Waypoints/cupx_reader.h
    ../../Source/Waypoints/cupx_reader.cpp
    ../../Source/utils/zzip_disk_file_stream.h
    ../../Source/utils/zzip_disk_file_stream.cpp
    ../../Source/utils/zzip_mem_disk.h
    ../../Source/utils/zzip_mem_disk.cpp
    ../../Source/Library/cpp-mmf/memory_mapped_file.cpp
)

# Lier les bibliothèques
target_include_directories(cupx_reader PRIVATE
    ${ZZIP_INCLUDE_DIRS}
    ${ZZIPMAP_INCLUDE_DIRS}
    ../../Source/Library/cpp-mmf
    ../../Source/
    ../../Source/xcs/
)

target_link_libraries(cupx_reader
    ${ZZIP_LIBRARIES}
    ${ZZIPMAP_LIBRARIES}
)

# Options de compilation
target_compile_options(cupx_reader PRIVATE -Wall -Wextra)

# Informations de compilation
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "zziplib include dirs: ${ZZIP_INCLUDE_DIRS}")
message(STATUS "zziplib libraries: ${ZZIP_LIBRARIES}")
message(STATUS "zzipmmapped include dirs: ${ZZIPMAP_INCLUDE_DIRS}")
message(STATUS "zzipmmapped libraries: ${ZZIPMAP_LIBRARIES}")

# Installation
install(TARGETS cupx_reader DESTINATION bin)

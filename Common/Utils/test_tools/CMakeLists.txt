cmake_minimum_required (VERSION 3.10)

project(test-tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost CONFIG REQUIRED COMPONENTS system)
message(STATUS "Boost version: ${Boost_VERSION}")

add_executable(lxnano-sim "lxnano-sim.cpp")
target_link_libraries(lxnano-sim ${Boost_SYSTEM_LIBRARY} pthread)

add_executable(krt2-ping "krt2-ping.cpp")
target_link_libraries(krt2-ping ${Boost_SYSTEM_LIBRARY} pthread)

add_executable(flarm-sim "flarm-sim.cpp")
target_link_libraries(flarm-sim ${Boost_SYSTEM_LIBRARY} pthread)

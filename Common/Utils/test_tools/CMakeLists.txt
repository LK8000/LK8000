cmake_minimum_required (VERSION 3.8)

project(test-tool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(lxnano-sim "lxnano-sim.cpp")
target_link_libraries(lxnano-sim boost_system)

add_executable(krt2-ping "krt2-ping.cpp")
target_link_libraries(krt2-ping boost_system pthread)




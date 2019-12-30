cmake_minimum_required (VERSION 3.8)

project(language-convert)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable (
	convert

	"language-convert.cpp"
)

target_include_directories( 
	convert 
	PRIVATE
	"../../Header"
)

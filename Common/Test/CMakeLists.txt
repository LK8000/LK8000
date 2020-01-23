cmake_minimum_required(VERSION 3.0)
project(LK8000_tests VERSION 0.0.1 LANGUAGES CXX)

find_package(doctest REQUIRED)
find_package(GeographicLib CONFIG REQUIRED)


include_directories(
	../Header
	../Source
	../Source/xcs
)

add_definitions(
	-DUSE_GDI
	-DNOMINMAX
	-DPOCO_NO_AUTOMATIC_LIBS
)

if(MSVC)
	foreach(flag_var
		    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	   if(${flag_var} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	   endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
	

endif()


add_subdirectory( ../Source/Library/poco Library)


# Make test executable
add_executable(
	LK8000_tests 

	../Source/Library/StringFunctions.cpp
	../Source/Waypoints/CUPToLatLon.cpp
	../Source/Waypoints/ReadAltitude.cpp
	../Source/Waypoints/ParseCUP.cpp
	../Source/Comm/GpsWeekNumberFix.cpp


	temp_global.cpp
	CUPToLatLon_test.cpp
	StrToDouble_test.cpp
	GpsWeekNumberFix_test.cpp
)

target_compile_features(LK8000_tests PRIVATE cxx_std_11)

target_link_libraries(LK8000_tests PRIVATE GeographicLib_STATIC GeographicLib::GeographicLib_STATIC)
target_link_libraries(LK8000_tests PRIVATE doctest::doctest)
target_link_libraries(LK8000_tests PRIVATE LK8000_POCO)

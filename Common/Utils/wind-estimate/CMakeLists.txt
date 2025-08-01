cmake_minimum_required (VERSION 3.10)

project(wind-estimate)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};/usr/share/cmake/geographiclib")

find_package(GeographicLib REQUIRED)


add_definitions(
    -DBOOST_NO_IOSTREAM
    -DBOOST_NO_IOSFWD
    -DBOOST_NO_STRINGSTREAM
    -DBOOST_NO_WSTREAMBUF
    -D__linux__
    -DHAVE_POSIX
    -D__STDC_FORMAT_MACROS
    -DENABLE_OPENGL
    -DHAVE_GLES
    -DHAVE_GLES2
    -DUSE_GLSL
    -DGL_GLEXT_PROTOTYPES
    -DUSE_EGL
    -DPOCO_NO_UNWINDOWS
    -DGLM_FORCE_RADIANS
    -DGLM_FORCE_SIZE_T_LENGTH
    -DUSE_CURL
	-DUSE_POLL_EVENT
	-DUSE_FREETYPE
    -DDOCTEST_CONFIG_DISABLE
    -DDEBUG_WIND
)


add_executable (
	wind-estimate

	"../../Source/Calc/windanalyser.cpp"
	"../../Source/Calc/windstore.cpp"
	"../../Source/Calc/windmeasurementlist.cpp"
    "../../Source/units.cpp"
    "../../Source/Library/Utm.cpp"
    "../../Source/xcs/Util/UTF8.cpp"

	"parse_igc.cpp"
	"mock.cpp"
	"wind-estimate.cpp"
)

target_include_directories( 
	wind-estimate
	PRIVATE
	${GeographicLib_INCLUDE_DIRS}
	"../../Header"
	"../../Header/linuxcompat"
	"../../Source"
	"../../Source/xcs"
)

target_compile_definitions(wind-estimate PRIVATE ${GeographicLib_DEFINITIONS})
target_link_libraries     (wind-estimate         ${GeographicLib_LIBRARIES})

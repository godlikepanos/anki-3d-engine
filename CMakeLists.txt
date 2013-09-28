CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(anki)

#
# Determin the system to build for. Do that first
#

if(WIN32)
	if(NOT WINDOWS)
		set(WINDOWS TRUE)
		message("++ Building for windows")
	endif()
elseif(UNIX AND NOT APPLE)
	if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
		if(NOT ANDROID)
			set(LINUX TRUE)
			message("++ Building for Linux")
		else()
			message("++ Building for Android")
		endif()
	else()
		message(FATAL_ERROR "Unknown unix")
	endif()
elseif(APPLE)
	if(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*")
		set(MACOS TRUE)
		message("++ Building for MacOS")
	else()
		message(FATAL_ERROR "Unknown apple")
	endif()
else()
	message(FATAL_ERROR "Unknown system")
endif()

#
# Configuration
#

set(ANKI_BUILD_TYPE "Release" CACHE STRING "Like CMAKE_BUILD_TYPE (Release or Debug)")

option(ANKI_BUILD_TOOLS "Build tools" ON)
option(ANKI_BUILD_TESTS "Build unit tests" OFF)
option(ANKI_BUILD_TESTAPP "Build test application" ON)
option(ANKI_BUILD_BENCH "Build benchmark application" OFF)

option(ANKI_WITH_GPERFTOOLS_PROF "Link with gperftools profiler" OFF)

option(ANKI_STRIP "Srip the symbols from the executables" OFF)

option(ANKI_ENABLE_COUNTERS "Enable performance counters. Small overhead" OFF)
if(ANKI_ENABLE_COUNTERS)
	set(_ANKI_ENABLE_COUNTERS 1)
else()
	set(_ANKI_ENABLE_COUNTERS 0)
endif()

# Address space
set(ANKI_CPU_ADDR_SPACE "0" CACHE STRING "The CPU architecture (0 or 32 or 64). If zero go native")

# SIMD
option(ANKI_ENABLE_SIMD "Enable or not SIMD optimizations" ON)

if(ANKI_ENABLE_SIMD)
	set(_ANKI_ENABLE_SIMD 1)
else()
	set(_ANKI_ENABLE_SIMD 0)
endif()

# Take a wild guess on the windowing system
if(LINUX)
	#set(_WIN_BACKEND "GLXX11")
	set(_WIN_BACKEND "SDL")
else(WINDOWS)
	set(_WIN_BACKEND "SDL")
elseif(ANDROID)
	set(_WIN_BACKEND "ANDROID")
elseif(MACOS)
	set(_WIN_BACKEND "SDL")
else()
	message(FATAL_ERROR "Couldn't determine the window backend. You need to specify it manually")
endif()

set(ANKI_WINDOW_BACKEND "${_WIN_BACKEND}" CACHE STRING "The window backend (GLXX11 or EGLX11 or EGLFBDEV or ANDROID or SDL or DUMMY)")

option(ANKI_GCC_TO_STRING_WORKAROUND "Enable workaround for C++11 GCC bug" OFF)
if(ANKI_GCC_TO_STRING_WORKAROUND)
	set(_ANKI_GCC_TO_STRING_WORKAROUND 1)
else()
	set(_ANKI_GCC_TO_STRING_WORKAROUND 0)
endif()

# Extra directories
set(ANKI_EXTRA_INCLUDE_DIRS CACHE STRING "Some extra include paths (Needed for some weird builds)")
set(ANKI_EXTRA_LIB_DIRS CACHE STRING "Some extra lib paths (Needed for some weird builds)")

# Valgrind
option(ANKI_VALGRIND_HAPPY "Make valgrind happy" OFF)

#
# Options that affect anki and extern
#

set(CXX_FLAGS "")
set(COMPILER_FLAGS "")
set(LINKER_FLAGS "")

# address space
if(NOT ANKI_CPU_ADDR_SPACE STREQUAL "0")
	set(LINKER_FLAGS "${LINKER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
endif()

# static libstdc++
set(CXX_FLAGS "${CXX_FLAGS} -static-libstdc++ ")

# SSE
if(LINUX OR MACOS OR WINDOWS)
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -msse4 ")
else()
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -mfpu=neon ")
endif()

if(${ANKI_BUILD_TYPE} STREQUAL "Debug")
	# Debug

	set(COMPILER_FLAGS "${COMPILER_FLAGS} -g3 -O0 ")
else()
	# Release

	# -flto ?
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -ffast-math -O4 -DNODEBUG ")

	# Add this because Android compiler complains
	if(ANDROID)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-data-sections ")
	endif()

	set(CXX_FLAGS "${CXX_FLAGS} -fno-rtti ")
endif()

# Valgrind hacks
if(ANKI_VALGRIND_HAPPY)
	add_definitions("-DGLIBCXX_FORCE_NEW")
endif()

# Disable GLU in GLEW
add_definitions(-DGLEW_NO_GLU)

# Strip
if(ANKI_STRIP)
	set(LINKER_FLAGS "${LINKER_FLAGS} -s ")
	set(COMPILER_FLAGS "${COMPILER_FLAGS} -s ")
endif()

# gperftools
if(ANKI_WITH_GPERFTOOLS_PROF)
	LINK_DIRECTORIES("/home/godlike/src/more/gperftools/install/lib")
	set(ANKI_GPERFTOOLS_LIBS "profiler")
else()
	set(ANKI_GPERFTOOLS_LIBS "")
endif()

include_directories(${ANKI_EXTRA_INCLUDE_DIRS})
link_directories(${ANKI_EXTRA_LIB_DIRS})

# Set the flags to cmake now
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

#
# Install
#
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/" CACHE PATH "The subdirectory to the header prefix")
set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install path")

message("++ Include install dir: ${INCLUDE_INSTALL_DIR}")
message("++ Lib install dir: ${LIB_INSTALL_DIR}")

#
# First the extern
#
add_subdirectory(extern)

#
# Doxygen
#
find_package(Doxygen)

if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
	
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

#
# Revision
#
find_package(Subversion 1.6)

if(Subversion_FOUND)
	Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} ER)
	set(ANKI_REVISION ${ER_WC_REVISION})
else()
	set(ANKI_REVISION "-1")
endif()

# 
# Config.h
#
set(ANKI_VERSION_MAJOR 0)
set(ANKI_VERSION_MINOR 1)
message("++ AnKi version: ${ANKI_VERSION_MAJOR}.${ANKI_VERSION_MINOR}")

if(${ANKI_BUILD_TYPE} STREQUAL "Debug")
	set(ANKI_DEBUG 1)
else()
	set(ANKI_DEBUG 0)
endif()

CONFIGURE_FILE("include/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h" DESTINATION "${INCLUDE_INSTALL_DIR}/anki")

#
# Include & lib directories
#
include_directories("extern/tinyxml2/include" "extern/lua" "extern/z" "extern/bullet" "extern/SDL2/include" "include" "${CMAKE_CURRENT_BINARY_DIR}")

if(LINUX OR MACOS OR WINDOWS)
	include_directories("extern/GLEW/include")
else()
	#include_directories("extern/GLES3/include")
endif()

if(ANDROID)
	include_directories("${ANDROID_NDK}/sources/android/native_app_glue")
endif()

#
# AnKi specific compiler flags
#

# thread local
add_definitions("-Dthread_local=__thread")

# AnKi compiler flags (Mainly warnings)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -W -Wextra -Wwrite-strings -Wunused -Wunused-variable -Wno-unused-parameter -Wundef -std=c++11 ")

#
# Add anki related dirs
#
add_subdirectory(src)

if(ANKI_BUILD_TESTS)
	ENABLE_TESTING()
	add_subdirectory(tests)
endif()

if(ANKI_BUILD_TOOLS)
	add_subdirectory(tools)
endif()

if(ANKI_BUILD_TESTAPP)
	add_subdirectory(testapp)
endif()

if(ANKI_BUILD_BENCH)
	add_subdirectory(bench)
endif()

#
# XXX
#
if(${ANDROID})
	# 1. generate Android.mk
	FILE(WRITE ./jni/Android.mk "APP_ABI := ${ANDROID_NDK_ABI_NAME}\n")

	# 2. generate gdb.setup
	GET_DIRECTORY_PROPERTY(include_directories DIRECTORY . include_directories)
	STRING(REGEX REPLACE ";" " " include_directories "${include_directories}")
	FILE(WRITE ./libs/${ANDROID_NDK_ABI_NAME}/gdb.setup "set solib-search-path ./libs/${ANDROID_NDK_ABI_NAME}\n")
	FILE(APPEND ./libs/${ANDROID_NDK_ABI_NAME}/gdb.setup "directory ${include_directories} ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}\n")

	# 3. copy gdbserver executable
	FILE(COPY ${ANDROID_NDK}/prebuilt/android-arm/gdbserver/gdbserver DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_NDK_ABI_NAME}/)

	set(LIBRARY_NAME ankibench)

	# 4. copy lib to obj
	#ADD_CUSTOM_COMMAND(TARGET ${LIBRARY_NAME} POST_BUILD COMMAND mkdir -p ./obj/local/${ANDROID_NDK_ABI_NAME}/)
	#ADD_CUSTOM_COMMAND(TARGET ${LIBRARY_NAME} POST_BUILD COMMAND cp ./libs/${ANDROID_NDK_ABI_NAME}/lib${LIBRARY_NAME}.so ./obj/local/${ANDROID_NDK_ABI_NAME}/)
endif()

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ANKI_PROJ)

#
# Configuration
#

OPTION(ANKI_BUILD_TOOLS "Build tools" ON)
OPTION(ANKI_BUILD_TESTS "Build Unit Tests" OFF)
OPTION(ANKI_BUILD_TESTAPP "Build Test Application" ON)

OPTION(ANKI_WITH_GPERFTOOLS_PROF "Link with gperftools profiler" OFF)

OPTION(ANKI_STRIP "Srip the symbols from the executables" OFF)

OPTION(ANKI_ENABLE_COUNTERS "Enable performance counters. Small overhead" OFF)
IF(ANKI_ENABLE_COUNTERS)
	SET(_ANKI_ENABLE_COUNTERS 1)
ELSE()
	SET(_ANKI_ENABLE_COUNTERS 0)
ENDIF()

# Because we want different compiler flags for each platform
SET(ANKI_CPU "X86" CACHE STRING "The CPU arch (X86 or ARM)")

# Address space
IF(${ANKI_CPU} STREQUAL "ARM")
	SET(_ADDR_SPACE "32")
ELSE()
	SET(_ADDR_SPACE "0")
ENDIF()

SET(ANKI_CPU_ADDR_SPACE "${_ADDR_SPACE}" CACHE STRING "The CPU architecture (0 or 32 or 64). If zero go native")

# SIMD
OPTION(ANKI_ENABLE_MATH_SIMD "Enable or not math SIMD optimizations" ON)
IF(ANKI_ENABLE_MATH_SIMD)
	IF(ANKI_CPU STREQUAL "X86")
		SET(ANKI_MATH_SIMD "SSE")
	ELSEIF(ANKI_CPU STREQUAL "ARM")
		SET(ANKI_MATH_SIMD "NEON")
	ELSE()
		MESSAGE(FATAL "Wrong ANKI_CPU set")
	ENDIF()
ELSE()
	SET(ANKI_MATH_SIMD "NONE")
ENDIF()

# Take a wild guess on the windowing system
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(_WIN_BACKEND "GLXX11")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	SET(_WIN_BACKEND "MACOS")
ELSE()
	SET(_WIN_BACKEND "DONT_KNOW")
ENDIF()

SET(ANKI_WINDOW_BACKEND "${_WIN_BACKEND}" CACHE STRING "The window backend (GLXX11 or EGLX11 or EGLFBDEV or MACOS or DUMMY)")

SET(ANKI_GCC_TO_STRING_WORKAROUND "0" CACHE STRING "Enable workaround for C++11 GCC bug (0 or 1)")

# Extra directories
SET(ANKI_EXTRA_INCLUDE_DIRS CACHE STRING "Some extra include paths (Needed for some weird builds)")
SET(ANKI_EXTRA_LIB_DIRS CACHE STRING "Some extra lib paths (Needed for some weird builds)")

# Valgrind
OPTION(ANKI_VALGRIND_HAPPY "Make valgrind happy" OFF)

#
# Options that affect anki and extern
#

SET(CXX_FLAGS "")
SET(C_FLAGS "")
SET(COMMON_FLAGS "")
SET(LINKER_FLAGS "")

# address space
IF(NOT ANKI_CPU_ADDR_SPACE STREQUAL "0")
	SET(LINKER_FLAGS "${LINKER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
	SET(COMMON_FLAGS "${COMMON_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
ENDIF()

# static libstdc++
SET(CXX_FLAGS "${CXX_FLAGS} -static-libstdc++ ")

# SSE
IF(ANKI_CPU STREQUAL "X86")
	SET(COMMON_FLAGS "${COMMON_FLAGS} -msse4 ")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Debug

	SET(COMMON_FLAGS "${COMMON_FLAGS} -g3 -O0 ")
ELSE()
	# Release

	SET(COMMON_FLAGS "${COMMON_FLAGS} -ffast-math -O4 ")

	SET(CXX_FLAGS "${CXX_FLAGS} -fno-rtti ")
ENDIF()

# Valgrind hacks
IF(ANKI_VALGRIND_HAPPY)
	ADD_DEFINITIONS("-DGLIBCXX_FORCE_NEW")
ENDIF()

# Disable GLU in GLEW
ADD_DEFINITIONS(-DGLEW_NO_GLU)

# Strip
IF(ANKI_STRIP)
	SET(LINKER_FLAGS "${LINKER_FLAGS} -s ")
	SET(COMMON_FLAGS "${COMMON_FLAGS} -s ")
ENDIF()

# gperftools
IF(ANKI_WITH_GPERFTOOLS_PROF)
	LINK_DIRECTORIES("/home/godlike/src/more/gperftools/install/lib")
	SET(ANKI_GPERFTOOLS_LIBS "profiler")
ELSE()
	SET(ANKI_GPERFTOOLS_LIBS "")
ENDIF()

INCLUDE_DIRECTORIES(${ANKI_EXTRA_INCLUDE_DIRS})
LINK_DIRECTORIES(${ANKI_EXTRA_LIB_DIRS})

# Set the flags to cmake now
SET(CMAKE_CXX_FLAGS "${COMMON_FLAGS} ${CXX_FLAGS}")
SET(CMAKE_C_FLAGS "${COMMON_FLAGS} ${C_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}")

#
# Install
#
SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/" CACHE PATH "The subdirectory to the header prefix")
SET(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install path")

MESSAGE("++ Include install dir: ${INCLUDE_INSTALL_DIR}")
MESSAGE("++ Lib install dir: ${LIB_INSTALL_DIR}")

#
# First the extern
#
ADD_SUBDIRECTORY(extern)

#
# Doxygen
#
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
	
	ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
		COMMENT "Generating API documentation with Doxygen" VERBATIM)
ENDIF()

#
# Revision
#
FIND_PACKAGE(Subversion 1.6 REQUIRED)

IF(Subversion_FOUND)
	Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} ER)
	SET(ANKI_REVISION ${ER_WC_REVISION})
ELSE()
	SET(ANKI_REVISION "-1")
ENDIF()

# 
# Config.h
#
SET(ANKI_VERSION_MAJOR 0)
SET(ANKI_VERSION_MINOR 1)
MESSAGE("++ AnKi version: ${ANKI_VERSION_MAJOR}.${ANKI_VERSION_MINOR}")

IF(CMAKE_BUILD_TYPE STREQUAL Debug)
	SET(ANKI_DEBUG 1)
ELSE()
	SET(ANKI_DEBUG 0)
ENDIF()

CONFIGURE_FILE("include/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h" DESTINATION "${INCLUDE_INSTALL_DIR}/anki")

#
# Include & lib directories
#
INCLUDE_DIRECTORIES("extern/tinyxml2/include" "extern/lua" "extern/png" "extern/z" "extern/bullet" "include" "${CMAKE_CURRENT_BINARY_DIR}")

IF(ANKI_WINDOW_BACKEND STREQUAL "GLXX11" OR ANKI_WINDOW_BACKEND STREQUAL "MACOS")
	INCLUDE_DIRECTORIES("extern/GLEW/include")
ELSE()
	INCLUDE_DIRECTORIES("extern/GLES3/include")
ENDIF()

#
# AnKi specific compiler flags
#

# thread local
ADD_DEFINITIONS("-Dthread_local=__thread")

# AnKi compiler flags (Mainly warnings)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -W -Wextra -Wwrite-strings -Wunused -Wunused-variable -Wno-unused-parameter -Wundef -std=c++11 ")

#
# Add anki related dirs
#
ADD_SUBDIRECTORY(src)

IF(ANKI_BUILD_TESTS)
	ENABLE_TESTING()
	ADD_SUBDIRECTORY(tests)
ENDIF()

IF(ANKI_BUILD_TOOLS)
	ADD_SUBDIRECTORY(tools)
ENDIF()

IF(ANKI_BUILD_TESTAPP)
	ADD_SUBDIRECTORY(testapp)
ENDIF()


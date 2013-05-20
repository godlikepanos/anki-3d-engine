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

# Take a wild guess on CPU address space
IF(${ANKI_CPU} STREQUAL "ARM")
	SET(_ADDR_SPACE "32")
ELSE()
	SET(_ADDR_SPACE "0")
ENDIF()

SET(ANKI_CPU_ADDR_SPACE "${_ADDR_SPACE}" CACHE STRING "The CPU architecture (0 or 32 or 64). If zero go native")
OPTION(ANKI_ENABLE_MATH_SIMD "Enable or not math SIMD optimizations" ON)

# libz and libpng
OPTION(ANKI_SYSTEM_LIBZ "Use the system's libz" OFF)
OPTION(ANKI_SYSTEM_LIBPNG "Use the system's libpng" OFF)

# Take a wild guess on the windowing system
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(_WIN_BACKEND "GLXX11")
ELSE()
	SET(_WIN_BACKEND "DONT_KNOW")
ENDIF()

SET(ANKI_WINDOW_BACKEND "${_WIN_BACKEND}" CACHE STRING "The window backend (GLXX11 or EGLX11 or EGLFBDEV or DUMMY)")

SET(ANKI_GCC_TO_STRING_WORKAROUND "0" CACHE STRING "Enable workaround for C++11 GCC bug (0 or 1)")

# Extra directories
SET(ANKI_EXTRA_INCLUDE_DIRS CACHE STRING "Some extra include paths (Needed for some weird builds)")
SET(ANKI_EXTRA_LIB_DIRS CACHE STRING "Some extra lib paths (Needed for some weird builds)")

#
# Options that affect anki and extern
#

# CPU
MESSAGE("++ Build for CPU: ${ANKI_CPU}")

IF(ANKI_CPU STREQUAL "X86")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4 ")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4 ")
ELSE()
ENDIF()

IF(ANKI_ENABLE_MATH_SIMD)
	IF(ANKI_CPU STREQUAL "X86")
		SET(ANKI_MATH_SIMD "SSE")
	ELSEIF(ANKI_CPU STREQUAL "ARM")
		SET(ANKI_MATH_SIMD "NEON")
	ELSE()
		MESSAGE(FATAL "Wrong ANKI_CPU set")
	ENDIF()

	MESSAGE("++ SIMD: true")
ELSE()
	SET(ANKI_MATH_SIMD "NONE")
	MESSAGE("++ SIMD: false")
ENDIF()

# CPU address space
IF(ANKI_CPU_ADDR_SPACE STREQUAL "0")
	MESSAGE("++ CPU address space: 0 (Native)")
ELSE()
	MESSAGE("++ CPU address space: ${ANKI_CPU_ADDR_SPACE}")
ENDIF()

IF(NOT ANKI_CPU_ADDR_SPACE STREQUAL "0")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m${ANKI_CPU_ADDR_SPACE} ")
ENDIF()

# platform
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(ANKI_PLATFORM "LINUX")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	SET(ANKI_PLATFORM "WINDOWS")
ENDIF()
MESSAGE("++ Platform: ${ANKI_PLATFORM}")

# libz and libpng
IF(ANKI_SYSTEM_LIBZ)
	SET(_ANKI_LIBZ "z")
	MESSAGE("++ Building with system libz")
ELSE()
	SET(_ANKI_LIBZ "ankiz")
	MESSAGE("++ Building with AnKi provided libz")
ENDIF()

IF(ANKI_SYSTEM_LIBPNG)
	SET(_ANKI_LIBPNG "png")
	MESSAGE("++ Building with system libpng")
ELSE()
	SET(_ANKI_LIBPNG "ankipng")
	MESSAGE("++ Building with AnKi provided libpng")
ENDIF()

# Disable GLU in GLEW
ADD_DEFINITIONS(-DGLEW_NO_GLU)

# Window backend
MESSAGE("++ Window backend: ${ANKI_WINDOW_BACKEND}")

#
# Common compiler flags
#

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -static-libstdc++")

# Build type
IF(CMAKE_BUILD_TYPE STREQUAL Debug)
	# Do nothing
ELSE()
	SET(FLAGS " -ffast-math")

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS} -fno-rtti ")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
ENDIF()

# Strip
IF(ANKI_STRIP)
	MESSAGE("++ Stipping symbols: true")
	SET(FLAGS " -s ")

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
ELSE()
	MESSAGE("++ Stipping symbols: false")
ENDIF()

# gperftools
IF(ANKI_WITH_GPERFTOOLS_PROF)
	LINK_DIRECTORIES("/home/godlike/src/more/gperftools/install/lib")
	SET(ANKI_GPERFTOOLS_LIBS "profiler")
	MESSAGE("++ With gperftools profiler: true")
ELSE()
	SET(ANKI_GPERFTOOLS_LIBS "")
	MESSAGE("++ With gperftools profiler: false")
ENDIF()

INCLUDE_DIRECTORIES(${ANKI_EXTRA_INCLUDE_DIRS})
LINK_DIRECTORIES(${ANKI_EXTRA_LIB_DIRS})

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
	MESSAGE("++ Debug build: true")
	SET(ANKI_DEBUG 1)
ELSE()
	MESSAGE("++ Debug build: false")
	SET(ANKI_DEBUG 0)
ENDIF()

CONFIGURE_FILE("include/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h" DESTINATION "${INCLUDE_INSTALL_DIR}/anki")

#
# Defines
#
ADD_DEFINITIONS("-Dthread_local=__thread")

#
# AnKi compiler flags (Mainly warnings)
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors -pedantic -ansi -Wall -W -Wextra -Wwrite-strings -Wno-unused -Wno-unused-parameter -Wundef -Werror -std=c++11")

#
# Include & lib directories
#
INCLUDE_DIRECTORIES("extern/tinyxml2/include" "extern/lua" "extern/png" "extern/bullet" "include"
	"${CMAKE_CURRENT_BINARY_DIR}")

IF(ANKI_WINDOW_BACKEND STREQUAL "GLXX11")
	INCLUDE_DIRECTORIES("extern/GLEW/include")
ELSE()
	INCLUDE_DIRECTORIES("extern/GLES3/include")
ENDIF()

# Add a few compiler specific stuff 
IF(${CMAKE_CXX_COMPILER} MATCHES ".*clang\\+\\+$")	
	INCLUDE_DIRECTORIES("/opt/libcxx/include/c++/v1")
	LINK_DIRECTORIES("/opt/libcxx/lib")
ELSE()
	# Dont use it. It produces warnings with -std=c++0x
	#ADD_DEFINITIONS("-fsingle-precision-constant")
ENDIF()

#
# Valgrind hacks
#
OPTION(ANKI_VALGRIND_HAPPY "Make valgrind happy" OFF)

IF(ANKI_VALGRIND_HAPPY)
	ADD_DEFINITIONS("-DGLIBCXX_FORCE_NEW")
ENDIF()

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

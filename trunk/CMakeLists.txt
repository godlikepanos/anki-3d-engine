CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ANKI_PROJ)

#
# Options that affect anki and extern
#

# CPU
SET(ANKI_CPU "X86" CACHE STRING "The CPU arch (X86 or ARM)")
MESSAGE("++ AnKi CPU: ${ANKI_CPU}")
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

MESSAGE("++ AnKi SIMD: ${ANKI_MATH_SIMD}")

# Arch
SET(ANKI_ARCH "0" CACHE STRING "The CPU architecture (0 or 32 or 64). If zero go native")
MESSAGE("++ AnKi arch: ${ANKI_ARCH}")

IF(NOT ANKI_ARCH STREQUAL "0")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${ANKI_ARCH} ")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${ANKI_ARCH} ")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m${ANKI_ARCH} ")
ENDIF()

# platform
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(ANKI_PLATFORM "LINUX")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	SET(ANKI_PLATFORM "WINDOWS")
ENDIF()
MESSAGE("++ AnKi platform: ${ANKI_PLATFORM}")

# libz and libpng
OPTION(ANKI_SYSTEM_LIBZ "Use the system's libz" OFF)
OPTION(ANKI_SYSTEM_LIBPNG "Use the system's libpng" OFF)

IF(ANKI_SYSTEM_LIBZ)
	SET(_ANKI_LIBZ "z")
	MESSAGE("++ Building with system libz")
ELSE()
	SET(_ANKI_LIBZ "ankiz")
	MESSAGE("++ Building with AnKi libz")
ENDIF()

IF(ANKI_SYSTEM_LIBPNG)
	SET(_ANKI_LIBPNG "png")
	MESSAGE("++ Building with system libpng")
ELSE()
	SET(_ANKI_LIBPNG "ankipng")
	MESSAGE("++ Building with AnKi libpng")
ENDIF()

# Window backend
SET(ANKI_WINDOW_BACKEND "GLXX11" CACHE STRING "The window backend (GLXX11 or EGLX11 or DUMMY)")
MESSAGE("++ AnKi window backend: ${ANKI_WINDOW_BACKEND}")

#
# Common compiler flags
#

# Build type
IF(CMAKE_BUILD_TYPE STREQUAL Debug)
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ")
ENDIF()

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
	
	ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
ENDIF()

#
# Revision
#
FIND_PACKAGE(Subversion 1.6 REQUIRED)

IF(Subversion_FOUND)
	Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} ER)
	#ADD_DEFINITIONS("-DANKI_REVISION=${ER_WC_REVISION}")
	SET(ANKI_REVISION ${ER_WC_REVISION})
ELSE()
	#ADD_DEFINITIONS("-DANKI_REVISION=-1")
	SET(ANKI_REVISION "-1")
ENDIF()

# 
# Config.h
#
SET(ANKI_VERSION_MAJOR 0)
SET(ANKI_VERSION_MINOR 1)
#ADD_DEFINITIONS("-DANKI_VERSION_MAJOR=${ANKI_VERSION_MAJOR}")
#ADD_DEFINITIONS("-DANKI_VERSION_MINOR=${ANKI_VERSION_MINOR}")
MESSAGE("++ AnKi version: ${ANKI_VERSION_MAJOR}.${ANKI_VERSION_MINOR}")

#ADD_DEFINITIONS("-DANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}")
#ADD_DEFINITIONS("-DANKI_MATH_SIMD_${ANKI_MATH_SIMD}")

CONFIGURE_FILE("include/anki/Config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h")
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/anki/Config.h" DESTINATION "${INCLUDE_INSTALL_DIR}/anki")

#
# Defines
#
ADD_DEFINITIONS("-Dthread_local=__thread")

#
# Compiler flags
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors -pedantic -ansi -Wall -W -Wwrite-strings -Wno-unused -Werror -Wno-long-long -msse4 -std=c++11 ")

IF(CMAKE_BUILD_TYPE STREQUAL Debug)
	# Removed because they do not work with boost::regexpr and who knows what
	# else
	#ADD_DEFINITIONS("-D_GLIBCXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC")
ELSE()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=core2 -ffast-math ")
ENDIF()

#
# Include & lib directories
#
INCLUDE_DIRECTORIES("extern/GLEW/include" "extern/tinyxml2/include" "extern/lua" "extern/png" "extern/bullet" "include"
	"${CMAKE_CURRENT_BINARY_DIR}")

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

OPTION(ANKI_BUILD_TESTS "Build Unit Tests" OFF)
IF(ANKI_BUILD_TESTS)
	ENABLE_TESTING()
	ADD_SUBDIRECTORY(tests)
ENDIF()

OPTION(ANKI_BUILD_TESTAPP "Build Test Application" ON)
IF(ANKI_BUILD_TESTAPP)
	ADD_SUBDIRECTORY(testapp)
ENDIF()

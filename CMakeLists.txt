CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ANKI_PROJ)

#
# Macros
#

# Check if INCDIR and LIBDIR dirs exist and the file ONEINCFILE as well
MACRO(ANKI_ADD_LIB INCDIR LIBDIR ONEINCFILE)
	IF(NOT EXISTS ${INCDIR})
		MESSAGE(FATAL_ERROR "Directory does not exist: " ${INCDIR})
	ENDIF()
	
	IF(NOT EXISTS ${LIBDIR})
		MESSAGE(FATAL_ERROR "Directory does not exist: " ${LIBDIR})
	ENDIF()
	
	IF(NOT EXISTS ${ONEINCFILE})
		MESSAGE(FATAL_ERROR "File not found: " ${ONEINCFILE})
	ENDIF()
	
	INCLUDE_DIRECTORIES(${INCDIR})
	LINK_DIRECTORIES(${LIBDIR})
	
	MESSAGE("++ Found: ${ONEINCFILE}")
	MESSAGE("++ Adding include dir: ${INCDIR}")
	MESSAGE("++ Adding lib dir: ${LIBDIR}")
ENDMACRO()

#
# Arch
#
SET(ARCH 64 CACHE STRING "The architecture (-mXX)")

MESSAGE("++ Arch: ${ARCH}")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${ARCH} ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${ARCH} ")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m${ARCH} ")

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
# Libraries
#
#FIND_PACKAGE(PNG REQUIRED)
#FIND_PACKAGE(JPEG 62 REQUIRED)

FIND_PACKAGE(Boost 1.46 REQUIRED)
SET(Boost_USE_STATIC_LIBS ON)
IF(Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
	ADD_DEFINITIONS("-DHAS_BOOST")
ENDIF()

#
# Freetype
#
SET(FREETYPE_INCLUDE_DIR "/usr/include/freetype2" CACHE PATH "The directory that contains the Freetype headers")
SET(FREETYPE_LIBRARY_DIR "/usr/lib" CACHE PATH "The directory that contains the libfreetype.so")

ANKI_ADD_LIB(${FREETYPE_INCLUDE_DIR} ${FREETYPE_LIBRARY_DIR} ${FREETYPE_INCLUDE_DIR}/freetype/freetype.h)

#
# Bullet (Because FIND_PACKAGE(Bullet) sucks)
#
SET(BULLET_INCLUDE_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/include/" CACHE PATH "The directory that contains the bullet directory with the header files")
SET(BULLET_LIBRARY_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/lib${ARCH}" CACHE PATH "The directory that contains the Bullet (static) libraries")

ANKI_ADD_LIB(${BULLET_INCLUDE_DIR}/bullet ${BULLET_LIBRARY_DIR} ${BULLET_INCLUDE_DIR}/bullet/btBulletCollisionCommon.h)

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
	ADD_DEFINITIONS("-DANKI_REVISION=${ER_WC_REVISION}")
	SET(ANKI_REVISION ${ER_WC_REVISION})
ELSE()
	ADD_DEFINITIONS("-DANKI_REVISION=-1")
	SET(ANKI_REVISION "-1")
ENDIF()

# 
# Config.h
#
SET(ANKI_VERSION_MAJOR 0)
SET(ANKI_VERSION_MINOR 1)
ADD_DEFINITIONS("-DANKI_VERSION_MAJOR=${ANKI_VERSION_MAJOR}")
ADD_DEFINITIONS("-DANKI_VERSION_MINOR=${ANKI_VERSION_MINOR}")
MESSAGE("++ AnKi version: ${ANKI_VERSION_MAJOR}.${ANKI_VERSION_MINOR}")

SET(ANKI_WINDOW_BACKEND "GLXX11" CACHE STRING "The window backend (GLXX11 or EGLX11)")
ADD_DEFINITIONS("-DANKI_WINDOW_BACKEND_${ANKI_WINDOW_BACKEND}")
MESSAGE("++ AnKi window backend: ${ANKI_WINDOW_BACKEND}")

SET(ANKI_CPU "X86" CACHE STRING "The CPU arch (X86 or ARM)")
ADD_DEFINITIONS("-DANKI_CPU_${ANKI_CPU}")
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

ADD_DEFINITIONS("-DANKI_MATH_SIMD_${ANKI_MATH_SIMD}")
MESSAGE("++ AnKi math SIMD: ${ANKI_MATH_SIMD}")

IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	SET(ANKI_PLATFORM "LINUX")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	SET(ANKI_PLATFORM "WINDOWS")
ENDIF()
ADD_DEFINITIONS("-DANKI_PLATFORM_${ANKI_PLATFORM}")
MESSAGE("++ AnKi platform: ${ANKI_PLATFORM}")

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
INCLUDE_DIRECTORIES("extern/GLEW/include" "extern/tinyxml2/include" "extern/lua" "extern/png" "include")
INCLUDE_DIRECTORIES("extern/include") # XXX Remove that some day
LINK_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/extern/lib64") # XXX Remove that some day

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
OPTION(VALGRIND_HAPPY "Make valgrind happy" OFF)

IF(VALGRIND_HAPPY)
	ADD_DEFINITIONS("-DGLIBCXX_FORCE_NEW")
ENDIF()

#
# Add anki related dirs
#
ADD_SUBDIRECTORY(src)

OPTION(BUILD_TESTS "Build Unit Tests" OFF)
IF(BUILD_TESTS)
	ENABLE_TESTING()
	ADD_SUBDIRECTORY(tests)
ENDIF()

OPTION(BUILD_TESTAPP "Build Test Application" ON)
IF(BUILD_TESTAPP)
	ADD_SUBDIRECTORY(testapp)
ENDIF()

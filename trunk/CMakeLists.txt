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
SET(ARCH 64 CACHE STRING "The architecture")

SET(CMAKE_CXX_FLAGS -m${ARCH})
SET(CMAKE_EXE_LINKER_FLAGS -m${ARCH})

#
# Install
#
SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/" CACHE PATH "The subdirectory to the header prefix")
SET(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install path")

#
# SVN
#
FIND_PACKAGE(Subversion 1.6 REQUIRED)

IF(Subversion_FOUND)
	Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} ER)
	ADD_DEFINITIONS("-DANKI_REVISION=${ER_WC_REVISION}")
	SET(ANKI_REVISION ${ER_WC_REVISION})
ELSE()
	ADD_DEFINITIONS("-DANKI_REVISION=???")
	SET(ANKI_REVISION unkown)
ENDIF()

# 
# Version
#
SET(ANKI_VERSION 0.1)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/anki/Version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/anki/Version.h)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/anki/Version.h DESTINATION ${INCLUDE_INSTALL_DIR}/anki)
	
#
# Doxygen
#
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
	
	ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
ENDIF()

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
SET(FREETYPE_INCLUDE_DIR "/usr/include/freetype2" 
	CACHE PATH "The directory that contains the Freetype headers")
SET(FREETYPE_LIBRARY_DIR "/usr/lib" 
	CACHE PATH "The directory that contains the libfreetype.so")

ANKI_ADD_LIB(${FREETYPE_INCLUDE_DIR} ${FREETYPE_LIBRARY_DIR} ${FREETYPE_INCLUDE_DIR}/freetype/freetype.h)

#
# Python
#
SET(PYTHON_VER 2.7 CACHE STRING "The python version")
SET(PYTHON_INCLUDE_DIR "/usr/include/python${PYTHON_VER}" CACHE PATH "The directory that contains the Python.h")
SET(PYTHON_LIBRARY_DIR "/usr/lib" CACHE PATH "The directory that contains the libpython${PYTHON_VER}.so")

ANKI_ADD_LIB(${PYTHON_INCLUDE_DIR} ${PYTHON_LIBRARY_DIR} ${PYTHON_INCLUDE_DIR}/Python.h)

#
# Bullet (Because FIND_PACKAGE(Bullet) sucks)
#
SET(BULLET_INCLUDE_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/include/" CACHE PATH 
	"The directory that contains the bullet directory with the header files")
SET(BULLET_LIBRARY_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/lib${ARCH}" CACHE PATH 
	"The directory that contains the Bullet (static) libraries")

ANKI_ADD_LIB(${BULLET_INCLUDE_DIR}/bullet ${BULLET_LIBRARY_DIR} 
	${BULLET_INCLUDE_DIR}/bullet/btBulletCollisionCommon.h)

# Extern dir
#
ADD_SUBDIRECTORY(extern)

INCLUDE_DIRECTORIES(extern/GLEW/include extern/tinyxml2/include)
INCLUDE_DIRECTORIES(extern/include)
LINK_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/extern/lib64")

#
# Defines & flags
#
ADD_DEFINITIONS(-DANKI_MATH_INTEL_SIMD
	-Dthread_local=__thread -pedantic-errors -pedantic -ansi -Wall 
	-W -Wwrite-strings -Wno-unused -Wfatal-errors -Werror -Wno-long-long 
	-msse4 -std=c++0x)

# Add a few compiler specific stuff 
IF(${CMAKE_CXX_COMPILER} MATCHES ".*clang\\+\\+$")	
	INCLUDE_DIRECTORIES("/opt/libcxx/include/c++/v1")
	LINK_DIRECTORIES("/opt/libcxx/lib")
ELSE()
	# Dont use it. It produces warnings with -std=c++0x
	#ADD_DEFINITIONS("-fsingle-precision-constant")
ENDIF()

IF(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
	ADD_DEFINITIONS("-DANKI_PLATFORM_LINUX")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	ADD_DEFINITIONS("-DANKI_PLATFORM_WINDOWS")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL Debug)
	# Removed because they do not work with boost::regexpr and who knows what
	# else
	#ADD_DEFINITIONS("-D_GLIBCXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC")
ELSE()
	ADD_DEFINITIONS("-DBOOST_DISABLE_ASSERTS -mtune=core2 -ffast-math")
ENDIF()

INCLUDE_DIRECTORIES(${ANKI_PROJ_SOURCE_DIR})

#
# libanki
#
INCLUDE_DIRECTORIES(include)
ADD_SUBDIRECTORY(src)

#
# Unit tests
#
OPTION(BUILD_TESTS "Build Unit Tests" OFF)

IF(BUILD_TESTS)
	ENABLE_TESTING()
	
	SET(GTEST_INCLUDE_DIR "/usr/include" CACHE PATH "The directory that contains the gtest directory")
	SET(GTEST_LIBRARY_DIR "/usr/lib" CACHE PATH "The directory that contains the libgtest.so")

	ANKI_ADD_LIB(${GTEST_INCLUDE_DIR} ${GTEST_LIBRARY_DIR} ${GTEST_INCLUDE_DIR}/gtest/gtest.h)
	
	ADD_SUBDIRECTORY(tests)
ENDIF()

#
# testapp
#
OPTION(BUILD_TESTAPP "Build Test Application" ON)

IF(BUILD_TESTAPP)
	ADD_SUBDIRECTORY(testapp)
ENDIF()

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

SET(CMAKE_CXX_FLAGS "-m${ARCH} ")
SET(CMAKE_C_FLAGS "-m${ARCH} ")
SET(CMAKE_EXE_LINKER_FLAGS "-m${ARCH} ")

#
# Install
#
SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/" CACHE PATH "The subdirectory to the header prefix")
SET(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install path")
	
#
# Doxygen
#
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
	
	ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
ENDIF()

#
# Valgrind hacks
#
OPTION(VALGRIND_HAPPY "Make valgrind happy" OFF)

IF(VALGRIND_HAPPY)
	ADD_DEFINITIONS("-DGLIBCXX_FORCE_NEW")
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
# Bullet (Because FIND_PACKAGE(Bullet) sucks)
#
SET(BULLET_INCLUDE_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/include/" CACHE PATH 
	"The directory that contains the bullet directory with the header files")
SET(BULLET_LIBRARY_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/lib${ARCH}" CACHE PATH 
	"The directory that contains the Bullet (static) libraries")

ANKI_ADD_LIB(${BULLET_INCLUDE_DIR}/bullet ${BULLET_LIBRARY_DIR} 
	${BULLET_INCLUDE_DIR}/bullet/btBulletCollisionCommon.h)

#
# Add subdirs
#
ADD_SUBDIRECTORY(extern)
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

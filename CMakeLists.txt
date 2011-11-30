CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(ANKI_PROJ)

#
# Macros
#
MACRO(ANKI_ADD_LIB INCDIR LIBDIR ONEINCFILE)
	IF(NOT EXISTS ${INCDIR})
		MESSAGE(FATAL_ERROR "Directory does not exist: " ${INCDIR})
	ENDIF()
	
	IF(NOT EXISTS ${LIBDIR})
		MESSAGE(FATAL_ERROR "Directory does not exist: " ${LIBDIR})
	ENDIF()
	
	IF(NOT EXISTS ${ONEINCFILE})
		MESSAGE(FATAL_ERROR "File not found: " ${ONEINCFILE})
	ELSE()
		MESSAGE("Found: " ${ONEINCFILE})
	ENDIF()
	
	INCLUDE_DIRECTORIES(${INCDIR})
	LINK_DIRECTORIES(${LIBDIR})
ENDMACRO()

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
ELSE()
	ADD_DEFINITIONS("-DANKI_REVISION=???")
ENDIF()

# 
# Version
#
SET(ANKI_VERSION 0.1)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/anki/Version.h 
	${CMAKE_CURRENT_BINARY_DIR}/anki/Version.h)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/anki/Version.h 
	DESTINATION ${INCLUDE_INSTALL_DIR}/anki)
	
#
# Doxygen
#
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/docs/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
	
	ADD_CUSTOM_TARGET(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
ENDIF()

#
# Libraries
#
FIND_PACKAGE(PNG 1.2 REQUIRED)
FIND_PACKAGE(JPEG 62 REQUIRED)
FIND_PACKAGE(Freetype 2.4.4 REQUIRED)


#
# Freetype
#
SET(FREETYPE_INCLUDE_DIR "/usr/include/freetype2" CACHE PATH "The directory that contains the Freetype headers")
SET(FREETYPE_LIBRARY_DIR "/usr/lib" CACHE PATH "The directory that contains the libfreetype.so")

ANKI_ADD_LIB(${FREETYPE_INCLUDE_DIR} ${FREETYPE_LIBRARY_DIR} ${FREETYPE_INCLUDE_DIR}/freetype/freetype.h)

#
# Python
#
SET(PYTHON_VER 2.7)
SET(PYTHON_INCLUDE_DIR "/usr/include/python${PYTHON_VER}" CACHE PATH "The directory that contains the Python.h and the other headers")
SET(PYTHON_LIBRARY_DIR "/usr/lib" CACHE PATH "The directory that contains the libpython${PYTHON_VER}.so")

ANKI_ADD_LIB(${PYTHON_INCLUDE_DIR} ${PYTHON_LIBRARY_DIR} ${PYTHON_INCLUDE_DIR}/Python.h)

#
# Bullet (Because FIND_PACKAGE(Bullet) sucks)
#
SET(BULLET_INCLUDE_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/include/" CACHE PATH "The directory that contains the bullet directory with the header files")
SET(BULLET_LIBRARY_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/lib64/" CACHE PATH "The directory that contains the Bullet (static) libraries")

ANKI_ADD_LIB(${BULLET_INCLUDE_DIR}/bullet ${BULLET_LIBRARY_DIR} ${BULLET_INCLUDE_DIR}/bullet/btBulletCollisionCommon.h)

#
# GLEW
#
SET(GLEW_INCLUDE_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/include/" CACHE PATH "The directory that contains the GL directory with the header files")
SET(GLEW_LIBRARY_DIR "${ANKI_PROJ_SOURCE_DIR}/extern/lib64/" CACHE PATH "The directory that contains the GLEW (static) libraries")

ANKI_ADD_LIB(${GLEW_INCLUDE_DIR} ${GLEW_LIBRARY_DIR} ${GLEW_INCLUDE_DIR}/GL/glew.h)

#
# Defines & flags
#
ADD_DEFINITIONS("-DANKI_MATH_INTEL_SIMD -DGLEW_MX -pedantic-errors -pedantic -ansi -Wall -Winline -W -Wwrite-strings -Wno-unused -Wfatal-errors -Werror -Wno-long-long -msse4")

# Add a few compiler specific stuff 
IF(${CMAKE_CXX_COMPILER} MATCHES ".*clang\\+\\+$")	
	# Do nothing
ELSE()
	ADD_DEFINITIONS("-fsingle-precision-constant")
ENDIF()

IF(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
	ADD_DEFINITIONS("-DPLATFORM_LINUX")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	ADD_DEFINITIONS("-DPLATFORM_WINDOWS")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL Debug)
	ADD_DEFINITIONS("-D_GLIBCXX_DEBUG -D_GLIBXX_DEBUG_PEDANTIC")
ELSE()
	ADD_DEFINITIONS("-DBOOST_DISABLE_ASSERTS -mtune=core2 -ffast-math")
ENDIF()


#INCLUDE_DIRECTORIES("${ANKI_PROJ_SOURCE_DIR}/extern/include")
#INCLUDE_DIRECTORIES("/usr/include/python2.6")
#INCLUDE_DIRECTORIES("/usr/include/freetype2")
INCLUDE_DIRECTORIES(${ANKI_PROJ_SOURCE_DIR})

LINK_DIRECTORIES(${ANKI_PROJ_SOURCE_DIR}/extern/lib64)

#
# libanki
#
ADD_SUBDIRECTORY(anki)

#
# Unit tests
#
OPTION(BUILD_UNIT_TESTS "Build Unit Tests" OFF)

IF(BUILD_UNIT_TESTS)
	ENABLE_TESTING()
	ADD_SUBDIRECTORY(unit-tests)
ENDIF()

#
# testapp
#
OPTION(BUILD_TESTAPP "Build Test Application" ON)

IF(BUILD_TESTAPP)
	ADD_SUBDIRECTORY(testapp)
ENDIF()

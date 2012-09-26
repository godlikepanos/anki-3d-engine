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
# Version
#
SET(ANKI_VERSION_MINOR 0)
SET(ANKI_VERSION_MAJOR 1)
ADD_DEFINITIONS("-DANKI_VERSION_MINOR=${ANKI_VERSION_MINOR}")
ADD_DEFINITIONS("-DANKI_VERSION_MAJOR=${ANKI_VERSION_MAJOR}")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/../include/anki/Version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/anki/Version.h)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/anki/Version.h DESTINATION ${INCLUDE_INSTALL_DIR}/anki)

#
# Defines
#
ADD_DEFINITIONS("-DANKI_MATH_INTEL_SIMD")
ADD_DEFINITIONS("-Dthread_local=__thread")

IF(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
	ADD_DEFINITIONS("-DANKI_PLATFORM_LINUX")
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
	ADD_DEFINITIONS("-DANKI_PLATFORM_WINDOWS")
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
# Extern directories
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../extern/GLEW/include ${CMAKE_CURRENT_SOURCE_DIR}/../extern/tinyxml2/include ${CMAKE_CURRENT_SOURCE_DIR}/../extern/lua)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../extern/include)
LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../extern/lib64)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)

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
# Add anki sub libraries
#
SET(ANKI_SUB_DIRS script renderer scene ui event input physics resource core misc gl collision math util)

SET(ANKI_LIBS "")

FOREACH(TMP ${ANKI_SUB_DIRS})
	ADD_SUBDIRECTORY(${TMP})
	SET(ANKI_LIBS ${ANKI_LIBS} anki${TMP})
ENDFOREACH()

ADD_LIBRARY(anki)

TARGET_LINK_LIBRARIES(anki ${ANKI_LIBS} BulletSoftBody BulletDynamics BulletCollision LinearMath ankiglew ankitinyxml2 ankilua GLU GL jpeg SDL png freetype)

SET_TARGET_PROPERTIES(anki PROPERTIES LINKER_LANGUAGE CXX)

# Install
#
INSTALL(TARGETS anki DESTINATION ${LIB_INSTALL_DIR})

INSTALL(DIRECTORY ${ANKI_PROJECT_SOURCE_DIR}/anki DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h" PATTERN .svn EXCLUDE)

SET(ANKI_UTIL_SOURCES Assert.cpp Exception.cpp Functions.cpp StringList.cpp File.cpp Allocator.cpp Memory.cpp System.cpp)

IF(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	SET(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerPosix.cpp FilePosix.cpp)
ELSE()
	MESSAGE(FATAL "See file")
ENDIF()

ADD_LIBRARY(ankiutil ${ANKI_UTIL_SOURCES})

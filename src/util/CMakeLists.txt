SET(ANKI_UTIL_SOURCES Assert.cpp BinaryStream.cpp Exception.cpp Functions.cpp Scanner.cpp StringList.cpp)

IF(ANKI_PLATFORM STREQUAL "LINUX")
	SET(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerPosix.cpp FilesystemPosix.cpp)
ELSE()
	MESSAGE(FATAL "See file")
ENDIF()

ADD_LIBRARY(ankiutil ${ANKI_UTIL_SOURCES})

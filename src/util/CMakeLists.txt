set(ANKI_UTIL_SOURCES Assert.cpp Exception.cpp Functions.cpp File.cpp Memory.cpp System.cpp HighRezTimer.cpp Thread.cpp)

if(LINUX OR ANDROID OR MACOS)
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerPosix.cpp FilePosix.cpp ThreadPosix.cpp)
else()
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerWindows.cpp FileWindows.cpp)
endif()

add_library(ankiutil ${ANKI_UTIL_SOURCES})

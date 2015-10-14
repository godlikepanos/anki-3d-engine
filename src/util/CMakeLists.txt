set(ANKI_UTIL_SOURCES Assert.cpp Functions.cpp File.cpp Filesystem.cpp Memory.cpp System.cpp HighRezTimer.cpp Thread.cpp Hash.cpp Logger.cpp String.cpp)

if(LINUX OR ANDROID OR MACOS)
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerPosix.cpp FilesystemPosix.cpp ThreadPosix.cpp)
else()
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerWindows.cpp FilesystemWindows.cpp ThreadWindows.cpp)
endif()

add_library(ankiutil ${ANKI_UTIL_SOURCES})

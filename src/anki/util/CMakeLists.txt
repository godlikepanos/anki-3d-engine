set(ANKI_UTIL_SOURCES Assert.cpp Functions.cpp File.cpp Filesystem.cpp Memory.cpp System.cpp HighRezTimer.cpp ThreadPool.cpp ThreadHive.cpp Hash.cpp Logger.cpp String.cpp StringList.cpp)

if(LINUX OR ANDROID OR MACOS)
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerPosix.cpp FilesystemPosix.cpp ThreadPosix.cpp)
else()
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} HighRezTimerWindows.cpp FilesystemWindows.cpp ThreadWindows.cpp)
endif()

if(LINUX)
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} INotifyLinux.cpp)
elseif(WINDOWS)
	set(ANKI_UTIL_SOURCES ${ANKI_UTIL_SOURCES} INotifyWindows.cpp)
endif()

add_library(ankiutil ${ANKI_UTIL_SOURCES})

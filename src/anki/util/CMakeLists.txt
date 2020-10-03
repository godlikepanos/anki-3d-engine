set(SOURCES Assert.cpp Functions.cpp File.cpp Filesystem.cpp Memory.cpp System.cpp HighRezTimer.cpp ThreadPool.cpp
	ThreadHive.cpp Hash.cpp Logger.cpp String.cpp StringList.cpp Tracer.cpp Serializer.cpp Xml.cpp F16.cpp)

if(LINUX OR ANDROID OR MACOS)
	set(SOURCES ${SOURCES} HighRezTimerPosix.cpp FilesystemPosix.cpp ThreadPosix.cpp ProcessPosix.cpp)
else()
	set(SOURCES ${SOURCES} HighRezTimerWindows.cpp FilesystemWindows.cpp ThreadWindows.cpp ProcessWindows.cpp Win32Minimal.cpp)
endif()

if(LINUX)
	set(SOURCES ${SOURCES} INotifyLinux.cpp)
elseif(WINDOWS)
	set(SOURCES ${SOURCES} INotifyWindows.cpp)
endif()

foreach(F ${SOURCES})
	addAnkiSourceFiles("${CMAKE_CURRENT_SOURCE_DIR}/${F}")
endforeach()

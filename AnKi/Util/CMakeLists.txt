set(sources
	Assert.cpp
	Functions.cpp
	File.cpp
	Filesystem.cpp
	Memory.cpp
	System.cpp
	HighRezTimer.cpp
	ThreadPool.cpp
	ThreadHive.cpp
	Hash.cpp
	Logger.cpp
	String.cpp
	StringList.cpp
	Tracer.cpp
	Serializer.cpp
	Xml.cpp
	F16.cpp)

if(LINUX OR ANDROID OR MACOS)
	set(sources ${sources}
		HighRezTimerPosix.cpp
		FilesystemPosix.cpp
		ThreadPosix.cpp
		ProcessPosix.cpp)
else()
	set(sources ${sources}
		HighRezTimerWindows.cpp
		FilesystemWindows.cpp
		ThreadWindows.cpp
		ProcessWindows.cpp
		Win32Minimal.cpp)
endif()

if(LINUX OR ANDROID)
	set(sources ${sources} INotifyPosix.cpp)
elseif(WINDOWS)
	set(sources ${sources} INotifyWindows.cpp)
endif()

add_library(AnKiUtil ${sources})
target_compile_definitions(AnKiUtil PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiUtil AnKiTinyXml2)

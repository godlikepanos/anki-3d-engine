set(sources
	Assert.cpp
	Functions.cpp
	File.cpp
	Filesystem.cpp
	MemoryPool.cpp
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
	F16.cpp
	Process.cpp
	Thread.cpp)

if(LINUX OR ANDROID OR MACOS)
	set(sources ${sources}
		HighRezTimerPosix.cpp
		FilesystemPosix.cpp
		ThreadPosix.cpp)
else()
	set(sources ${sources}
		HighRezTimerWindows.cpp
		FilesystemWindows.cpp
		ThreadWindows.cpp
		Win32Minimal.cpp)
endif()

if(LINUX OR ANDROID)
	set(sources ${sources} INotifyPosix.cpp)
elseif(WINDOWS)
	set(sources ${sources} INotifyWindows.cpp)
endif()

file(GLOB_RECURSE headers *.h)

add_library(AnKiUtil ${sources} ${headers})
target_compile_definitions(AnKiUtil PRIVATE -DANKI_SOURCE_FILE)

set(libs AnKiTinyXml2)
if(LINUX OR WINDOWS)
	set(libs ${libs} reproc)
endif()

target_link_libraries(AnKiUtil ${libs})

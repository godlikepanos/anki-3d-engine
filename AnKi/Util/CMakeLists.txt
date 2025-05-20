set(sources
	Assert.cpp
	Functions.cpp
	File.cpp
	Filesystem.cpp
	MemoryPool.cpp
	System.cpp
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
	Thread.cpp
	Singleton.cpp
	ThreadJobManager.cpp
	CVarSet.cpp)

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

if(ANDROID)
	set(libs ${libs} AnKiStreamlineAnnotate)
endif()

# PIX annotations
if(DIRECTX AND ANKI_TRACE)
	if(ARM)
		set(cpu_arch "ARM64")
	elseif(X86)
		set(cpu_arch "x64")
	else()
		message(FATAL_ERROR "Unhandled case")
	endif()

	find_library(PIX_LIB WinPixEventRuntime HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/Pix/bin/${cpu_arch}")
	set(libs ${libs} ${PIX_LIB})

	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/Pix/include/WinPixEventRuntime")
endif()

target_link_libraries(AnKiUtil ${libs})

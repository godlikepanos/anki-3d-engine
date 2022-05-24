set(sources
	App.cpp
	ConfigSet.cpp
	GpuMemoryPools.cpp
	DeveloperConsole.cpp
	CoreTracer.cpp
	MaliHwCounters.cpp
	StatsUi.cpp)

if(ANKI_HEADLESS)
	set(sources ${sources} NativeWindowHeadless.cpp)
elseif(SDL)
	set(sources ${sources} NativeWindowSdl.cpp)
elseif(ANDROID)
	set(sources ${sources} NativeWindowAndroid.cpp)
else()
	message(FATAL_ERROR "Not implemented")
endif()

add_library(AnKiCore ${sources})

if(SDL)
	if(LINUX)
		set(extra_libs X11-xcb pthread dl)
	endif()

	set(extra_libs ${extra_libs} SDL2-static)
elseif(ANDROID)
	set(extra_libs log android AnKiAndroidNativeGlue AnKiHwcPipe)
elseif(WINDOWS)
	set(extra_libs version Imm32 Winmm DbgHelp)
endif()

target_compile_definitions(AnKiCore PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiCore AnKiGr AnKiResource AnKiUi AnKiRenderer AnKiUtil AnKiPhysics AnKiScript AnKiInput ${extra_libs})

set(sources
	App.cpp
	CoreTracer.cpp
	MaliHwCounters.cpp
	StatsSet.cpp
	GpuMemory/UnifiedGeometryBuffer.cpp
	GpuMemory/GpuSceneBuffer.cpp
	GpuMemory/RebarTransientMemoryPool.cpp
	GpuMemory/GpuReadbackMemoryPool.cpp
	GpuMemory/GpuVisibleTransientMemoryPool.cpp)

set(headers
	App.h
	Common.h
	CoreTracer.h
	MaliHwCounters.h
	StatsSet.h
	StdinListener.h
	GpuMemory/UnifiedGeometryBuffer.h
	GpuMemory/GpuSceneBuffer.h
	GpuMemory/RebarTransientMemoryPool.h
	GpuMemory/GpuReadbackMemoryPool.h)

add_library(AnKiCore ${sources} ${headers})

if(SDL)
	if(LINUX)
		set(extra_libs pthread dl)
	endif()

	set(extra_libs ${extra_libs} SDL2-static)
elseif(ANDROID)
	set(extra_libs log android nativewindow AnKiAndroidNativeGlue AnKiHwcPipe)
elseif(WINDOWS)
	set(extra_libs version Imm32 Winmm DbgHelp)
endif()

target_compile_definitions(AnKiCore PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiCore AnKiGr AnKiResource AnKiUi AnKiRenderer AnKiUtil AnKiPhysics AnKiScript AnKiWindow ${extra_libs})

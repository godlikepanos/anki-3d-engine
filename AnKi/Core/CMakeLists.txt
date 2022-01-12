set(SOURCES App.cpp ConfigSet.cpp GpuMemoryPools.cpp DeveloperConsole.cpp CoreTracer.cpp MaliHwCounters.cpp
	StatsUi.cpp)
file(GLOB HEADERS *.h)

if(ANKI_HEADLESS)
	set(SOURCES ${SOURCES} NativeWindowHeadless.cpp)
elseif(SDL)
	set(SOURCES ${SOURCES} NativeWindowSdl.cpp)
elseif(ANDROID)
	set(SOURCES ${SOURCES} NativeWindowAndroid.cpp)
else()
	message(FATAL_ERROR "Not implemented")
endif()

foreach(S ${SOURCES})
	anki_add_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${S}" ${HEADERS})
endforeach()

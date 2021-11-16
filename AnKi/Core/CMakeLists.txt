set(SOURCES App.cpp ConfigSet.cpp GpuMemoryPools.cpp DeveloperConsole.cpp CoreTracer.cpp)
file(GLOB HEADERS *.h)

if(SDL)
	set(SOURCES ${SOURCES} NativeWindowSdl.cpp)
elseif(ANDROID)
	set(SOURCES ${SOURCES} NativeWindowAndroid.cpp)
else()
	message(FATAL_ERROR "Not implemented")
endif()

foreach(S ${SOURCES})
	anki_add_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${S}" ${HEADERS})
endforeach()

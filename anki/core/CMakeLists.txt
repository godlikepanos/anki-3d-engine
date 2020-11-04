set(SOURCES App.cpp ConfigSet.cpp StagingGpuMemoryManager.cpp DeveloperConsole.cpp CoreTracer.cpp)
file(GLOB HEADERS *.h)

if(SDL)
	set(SOURCES ${SOURCES} NativeWindowSdl.cpp)
else()
	message(FATAL_ERROR "Not implemented")
endif()

foreach(S ${SOURCES})
	addAnkiSourceFiles("${CMAKE_CURRENT_SOURCE_DIR}/${S}" ${HEADERS})
endforeach()

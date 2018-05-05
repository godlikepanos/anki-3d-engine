set(SOURCES App.cpp Config.cpp StagingGpuMemoryManager.cpp)

if(SDL)
	set(SOURCES ${SOURCES} NativeWindowSdl.cpp)
else()
	message(FATAL_ERROR "Not implemented")
endif()

foreach(S ${SOURCES})
	addAnkiSourceFiles("${CMAKE_CURRENT_SOURCE_DIR}/${S}")
endforeach()

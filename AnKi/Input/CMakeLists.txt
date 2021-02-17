set(SOURCES Input.cpp)

if(SDL)
	set(SOURCES ${SOURCES} InputSdl.cpp)
else()
	set(SOURCES ${SOURCES} InputDummy.cpp)
endif()

foreach(F ${SOURCES})
	addAnkiSourceFiles("${CMAKE_CURRENT_SOURCE_DIR}/${F}")
endforeach()

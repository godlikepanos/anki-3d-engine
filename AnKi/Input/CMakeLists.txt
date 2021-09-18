set(SOURCES Input.cpp)

if(SDL)
	set(SOURCES ${SOURCES} InputSdl.cpp)
elseif(ANDROID)
set(SOURCES ${SOURCES} InputAndroid.cpp)
else()
	set(SOURCES ${SOURCES} InputDummy.cpp)
endif()

foreach(F ${SOURCES})
	anki_add_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${F}")
endforeach()

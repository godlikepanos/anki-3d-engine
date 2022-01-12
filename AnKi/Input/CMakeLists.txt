set(SOURCES "")

if(ANKI_HEADLESS)
	set(SOURCES ${SOURCES} InputDummy.cpp)
elseif(SDL)
	set(SOURCES ${SOURCES} InputSdl.cpp)
elseif(ANDROID)
	set(SOURCES ${SOURCES} InputAndroid.cpp)
endif()

foreach(F ${SOURCES})
	anki_add_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${F}")
endforeach()

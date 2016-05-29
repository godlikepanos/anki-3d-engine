file(GLOB ANKI_INPUT_SOURCES Input.cpp)

if(SDL)
	set(ANKI_INPUT_SOURCES ${ANKI_INPUT_SOURCES} InputSdl.cpp)
else()
	set(ANKI_INPUT_SOURCES ${ANKI_INPUT_SOURCES} InputDummy.cpp)
endif()

add_library(ankiinput ${ANKI_INPUT_SOURCES})
target_link_libraries(ankiinput ankicore)

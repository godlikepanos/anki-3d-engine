set(ANKI_CORE_SOURCES App.cpp StdinListener.cpp Config.cpp Trace.cpp StagingGpuMemoryManager.cpp)

if(SDL)
	set(ANKI_CORE_SOURCES ${ANKI_CORE_SOURCES} NativeWindowSdl.cpp)
else()
	message(FATAL_ERROR "Not implemented")
endif()

add_library(ankicore ${ANKI_CORE_SOURCES})

target_link_libraries(ankicore ankiscript ankirenderer ankiscene ankiresource ankiinput ankimisc ankiutil)

if(SDL)
	target_link_libraries(ankicore SDL2-static)
endif()

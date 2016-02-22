file(GLOB ANKI_GR_SOURCES *.cpp common/*.cpp)

if(GL)
	file(GLOB ANKI_GR_BACKEND_SOURCES gl/*.cpp)
else()
	file(GLOB ANKI_GR_BACKEND_SOURCES vulkan/*.cpp)
endif()

add_library(ankigr ${ANKI_GR_SOURCES} ${ANKI_GR_BACKEND_SOURCES})
target_link_libraries(ankigr ankiutil)

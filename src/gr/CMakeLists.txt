file(GLOB ANKI_GR_SOURCES *.cpp common/*.cpp)


if(GL)
	set(GR_BACKEND "gl")
else()
	set(GR_BACKEND "vulkan")
endif()

file(GLOB ANKI_GR_BACKEND_SOURCES ${GR_BACKEND}/*.cpp)
list(REMOVE_ITEM ANKI_GR_BACKEND_SOURCES "${GR_BACKEND}/GrManagerImplSdl.cpp")

if(${ANKI_WINDOW_BACKEND} STREQUAL "SDL")
	set(ANKI_GR_BACKEND_SOURCES ${ANKI_GR_BACKEND_SOURCES} "${GR_BACKEND}/GrManagerImplSdl.cpp")
else()
	message(FATAL "Missing backend")
endif()

add_library(ankigr ${ANKI_GR_SOURCES} ${ANKI_GR_BACKEND_SOURCES})
target_link_libraries(ankigr ankiutil)

file(GLOB_RECURSE GLEW_SOURCES *.c)
file(GLOB_RECURSE GLEW_HEADERS *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_definitions(-DGLEW_NO_GLU -Os)

if(WINDOWS)
	add_definitions("-DGLEW_STATIC")
endif()

add_library(ankiglew ${GLEW_SOURCES} ${GLEW_HEADERS})

if(WINDOWS)
	target_link_libraries(ankiglew opengl32)
endif()

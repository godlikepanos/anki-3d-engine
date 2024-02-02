file(GLOB_RECURSE SOURCES *.c)
file(GLOB_RECURSE HEADERS *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

add_library(AnKiStreamlineAnnotate ${SOURCES} ${HEADERS})
set_target_properties(AnKiStreamlineAnnotate PROPERTIES LINKER_LANGUAGE C)

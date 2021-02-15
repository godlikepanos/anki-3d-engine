file(GLOB_RECURSE ZLIB_SOURCES *.c)
file(GLOB_RECURSE ZLIB_HEADERS *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

add_definitions(-DUSE_FILE32API)

add_library(ankiz ${ZLIB_SOURCES} ${ZLIB_HEADERS})
set_target_properties(ankiz PROPERTIES LINKER_LANGUAGE C)

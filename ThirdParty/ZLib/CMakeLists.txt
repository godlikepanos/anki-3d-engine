file(GLOB_RECURSE ZLIB_SOURCES *.c)
file(GLOB_RECURSE ZLIB_HEADERS *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)

add_definitions(-DUSE_FILE32API)
if(GCC OR CLANG)
	add_compile_options(-Wno-implicit-function-declaration)
endif()

add_library(AnKiZLib ${ZLIB_SOURCES} ${ZLIB_HEADERS})
set_target_properties(AnKiZLib PROPERTIES LINKER_LANGUAGE C)

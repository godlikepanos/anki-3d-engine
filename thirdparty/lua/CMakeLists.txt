file(GLOB_RECURSE LUA_SOURCES *.c)
file(GLOB_RECURSE LUA_HEADERS *.h)

if(NOT WINDOWS)
	message("++ Using LUA POSIX")
	add_definitions("-DLUA_USE_POSIX")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(ankilua ${LUA_SOURCES} ${LUA_HEADERS})
#set_target_properties(ankilua PROPERTIES LINKER_LANGUAGE C COMPILE_FLAGS "-DLUA_USE_POSIX")


file(GLOB_RECURSE SOURCES *.cpp)

add_library(ankispirvcross ${SOURCES})

target_include_directories(ankispirvcross PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_compile_definitions(ankispirvcross PRIVATE SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)

if(GCC)
	target_compile_options(ankispirvcross PRIVATE -std=c++11)
endif()

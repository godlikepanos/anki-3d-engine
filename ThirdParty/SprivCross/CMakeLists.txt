file(GLOB_RECURSE SOURCES *.cpp)

add_library(AnKiSpirvCross ${SOURCES})

target_include_directories(AnKiSpirvCross PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_compile_definitions(AnKiSpirvCross PRIVATE SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)

if(GCC)
	target_compile_options(AnKiSpirvCross PRIVATE -std=c++11)
endif()

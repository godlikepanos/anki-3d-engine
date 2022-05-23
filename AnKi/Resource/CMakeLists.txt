file(GLOB_RECURSE sources *.cpp)
add_library(AnKiResource ${sources})
target_compile_definitions(AnKiResource PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiResource AnKiCore AnKiGr AnKiPhysics AnKiZLib AnKiShaderCompiler)

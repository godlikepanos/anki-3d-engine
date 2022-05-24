file(GLOB_RECURSE sources *.cpp)
add_library(AnKiShaderCompiler ${sources})
target_compile_definitions(AnKiShaderCompiler PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiShaderCompiler AnKiGrCommon AnKiSpirvCross glslang SPIRV OGLCompiler OSDependent)

anki_new_executable(ShaderDump ShaderProgramBinaryDumpMain.cpp)
target_link_libraries(ShaderDump AnKiShaderCompiler)

anki_new_executable(ShaderCompiler ShaderProgramCompilerMain.cpp)
target_link_libraries(ShaderCompiler AnKiShaderCompiler)

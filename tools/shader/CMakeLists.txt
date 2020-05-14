include_directories("../../src")

add_executable(shader_dump ShaderProgramBinaryDumpMain.cpp)
target_link_libraries(shader_dump anki)
installExecutable(shader_dump)

add_executable(shader_compiler ShaderProgramCompilerMain.cpp)
target_link_libraries(shader_compiler anki)
installExecutable(shader_compiler)

file(GLOB_RECURSE sources *.cpp)

include_directories("..")

anki_new_executable(Tests ${sources})
target_compile_definitions(Tests PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(Tests AnKi AnKiShaderCompiler AnKiImporter)

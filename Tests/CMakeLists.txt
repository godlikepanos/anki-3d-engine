file(GLOB_RECURSE sources *.cpp)

include_directories("..")

anki_new_executable(Tests ${sources})
target_compile_definitions(Tests PRIVATE -DANKI_SOURCE_FILE)

if(!ANDROID)
	set(extra_libs "AnKiImporter")
endif()

target_link_libraries(Tests AnKi AnKiShaderCompiler ${extra_libs})

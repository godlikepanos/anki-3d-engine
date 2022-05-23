file(GLOB_RECURSE sources *.cpp)
add_library(AnKiImporter ${sources})
target_compile_definitions(AnKiImporter PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiImporter AnKiResource)

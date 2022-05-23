file(GLOB_RECURSE sources *.cpp)
add_library(AnKiMath ${sources})
target_compile_definitions(AnKiMath PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiMath AnKiUtil)

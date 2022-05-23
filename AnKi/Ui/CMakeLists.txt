file(GLOB_RECURSE sources *.cpp)
add_library(AnKiUi ${sources})
target_compile_definitions(AnKiUi PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiUi AnKiGr AnKiResource AnKiImGui)

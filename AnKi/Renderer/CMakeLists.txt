file(GLOB_RECURSE sources *.cpp)
add_library(AnKiRenderer ${sources})
target_compile_definitions(AnKiRenderer PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiRenderer AnKiGr AnKiResource AnKiUi)

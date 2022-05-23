file(GLOB_RECURSE sources *.cpp)
add_library(AnKiScript ${sources})
target_compile_definitions(AnKiScript PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiScript AnKiRenderer AnKiScene AnKiMath AnKiLua)

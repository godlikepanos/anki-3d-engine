file(GLOB_RECURSE sources *.cpp)
add_library(AnKiScene ${sources})
target_compile_definitions(AnKiScene PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiScene AnKiResource AnKiScript AnKiCollision)

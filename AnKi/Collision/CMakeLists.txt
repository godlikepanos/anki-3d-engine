file(GLOB_RECURSE sources *.cpp)
add_library(AnKiCollision ${sources})
target_compile_definitions(AnKiCollision PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiCollision AnKiMath)

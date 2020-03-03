include_directories("../../src")

file(GLOB_RECURSE SOURCES *.cpp)

add_executable(shader_dump ${SOURCES})
target_link_libraries(shader_dump anki)
installExecutable(shader_dump)

include_directories("../../src")

add_definitions("-fexceptions")

file(GLOB_RECURSE SOURCES *.cpp)

add_executable(gltf_exporter ${SOURCES})
target_link_libraries(gltf_exporter anki)
installExecutable(gltf_exporter)

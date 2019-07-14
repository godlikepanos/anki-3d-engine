include_directories("../../src")

file(GLOB_RECURSE SOURCES *.cpp)

add_executable(gltf_exporter_2 ${SOURCES})
target_link_libraries(gltf_exporter_2 anki)
installExecutable(gltf_exporter_2)

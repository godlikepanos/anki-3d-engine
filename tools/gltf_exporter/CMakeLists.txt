include_directories("../../src")

add_definitions("-fexceptions")

add_executable(gltf_exporter Main.cpp Exporter.cpp)
target_link_libraries(gltf_exporter anki)
installExecutable(gltf_exporter)

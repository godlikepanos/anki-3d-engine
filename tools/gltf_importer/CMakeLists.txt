include_directories("../../src")

file(GLOB_RECURSE SOURCES *.cpp)

add_executable(gltf_importer ${SOURCES})
target_link_libraries(gltf_importer anki)
installExecutable(gltf_importer)

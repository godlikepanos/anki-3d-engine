file(GLOB_RECURSE SOURCES *.cpp)

add_executable(GltfImporter ${SOURCES})
target_link_libraries(GltfImporter AnKi)
installExecutable(GltfImporter)

file(GLOB_RECURSE sources *.cpp)

anki_new_executable(GltfImporter ${sources})
target_link_libraries(GltfImporter AnKiImporter AnKiMeshOptimizer)

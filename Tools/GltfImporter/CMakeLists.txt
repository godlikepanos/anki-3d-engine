file(GLOB_RECURSE SOURCES *.cpp)

anki_new_executable(GltfImporter ${SOURCES})
target_link_libraries(GltfImporter AnKi)

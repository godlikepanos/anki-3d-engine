include_directories("../../thirdparty/assimp/include")
include_directories("../../src")

add_definitions("-fexceptions")

add_executable(sceneimp Main.cpp Common.cpp Exporter.cpp ExporterMesh.cpp ExporterMaterial.cpp)
target_link_libraries(sceneimp ankiassimp anki)
installExecutable(sceneimp)

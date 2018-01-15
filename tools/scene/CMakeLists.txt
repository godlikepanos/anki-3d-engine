include_directories("../../thirdparty/assimp/include")

add_definitions("-fexceptions")

add_executable(sceneimp Main.cpp Common.cpp Exporter.cpp ExporterMesh.cpp ExporterMaterial.cpp)
target_link_libraries(sceneimp ankiassimp)
installExecutable(sceneimp)

include_directories("../../thirdparty/assimp/include")

add_definitions("-fexceptions")

add_executable(ankisceneimp Main.cpp Common.cpp Exporter.cpp ExporterMesh.cpp)
target_link_libraries(ankisceneimp ankiassimp) 

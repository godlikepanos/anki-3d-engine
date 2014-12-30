include_directories("../../thirdparty/assimp/include")

add_definitions("-fexceptions")

add_executable(ankisceneimp Main.cpp Model.cpp Animation.cpp Light.cpp 
	Common.cpp Exporter.cpp)
target_link_libraries(ankisceneimp ankiassimp) 

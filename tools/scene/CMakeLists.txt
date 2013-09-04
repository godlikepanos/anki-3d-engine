INCLUDE_DIRECTORIES("../../extern/assimp/include")
	
ADD_EXECUTABLE(ankisceneimp Main.cpp)
TARGET_LINK_LIBRARIES(ankisceneimp ankiassimp) 

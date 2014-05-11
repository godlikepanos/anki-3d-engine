INCLUDE_DIRECTORIES("../../extern/assimp/include")
	
ADD_EXECUTABLE(ankisceneimp Main.cpp Model.cpp Animation.cpp Light.cpp 
	Common.cpp)
TARGET_LINK_LIBRARIES(ankisceneimp ankiassimp) 

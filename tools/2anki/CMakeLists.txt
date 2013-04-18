INCLUDE_DIRECTORIES("../../extern/assimp/include")
	
ADD_EXECUTABLE(2anki Main.cpp)
TARGET_LINK_LIBRARIES(2anki ankiassimp) 

file(GLOB SOURCES *.cpp utils/*.cpp)

if(GL)
	set(GR_BACKEND "gl")
else()
	set(GR_BACKEND "vulkan")
endif()

file(GLOB GR_BACKEND_SOURCES ${GR_BACKEND}/*.cpp)

addAnkiSourceFiles(${SOURCES})
addAnkiSourceFiles(${GR_BACKEND_SOURCES})

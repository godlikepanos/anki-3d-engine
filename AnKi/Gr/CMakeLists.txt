file(GLOB SOURCES *.cpp Utils/*.cpp)

if(GL)
	set(GR_BACKEND "Gl")
else()
	set(GR_BACKEND "Vulkan")
endif()

file(GLOB GR_BACKEND_SOURCES ${GR_BACKEND}/*.cpp)

addAnkiSourceFiles(${SOURCES})
addAnkiSourceFiles(${GR_BACKEND_SOURCES})

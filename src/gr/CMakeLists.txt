file(GLOB ANKI_GR_SOURCES *.cpp common/*.cpp)

if(GL)
	set(GR_BACKEND "gl")
	set(EXTRA_LIBS "")
else()
	set(GR_BACKEND "vulkan")
	set(EXTRA_LIBS GLSLANG_LIB SPIRV_LIB OSD_LIB GLC_LIB HLSL_LIB)
endif()

file(GLOB ANKI_GR_BACKEND_SOURCES ${GR_BACKEND}/*.cpp)
list(REMOVE_ITEM ANKI_GR_BACKEND_SOURCES "${GR_BACKEND}/GrManagerImplSdl.cpp")

if(SDL)
	set(ANKI_GR_BACKEND_SOURCES ${ANKI_GR_BACKEND_SOURCES} "${GR_BACKEND}/GrManagerImplSdl.cpp")
else()
	message(FATAL "Missing backend")
endif()

add_library(ankigr ${ANKI_GR_SOURCES} ${ANKI_GR_BACKEND_SOURCES})
target_link_libraries(ankigr ankiutil ankicore ${EXTRA_LIBS})

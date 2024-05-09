file(GLOB_RECURSE sources *.cpp)
file(GLOB_RECURSE headers *.h)
add_library(AnKiShaderCompiler ${sources} ${headers})
target_compile_definitions(AnKiShaderCompiler PRIVATE -DANKI_SOURCE_FILE)

if(WINDOWS)
	#set(libs ../../../ThirdParty/Dxc/dxcompiler)
endif()

target_link_libraries(AnKiShaderCompiler AnKiGrCommon AnKiSpirvCross SPIRV-Tools ${libs})

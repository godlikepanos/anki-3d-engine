set(common_sources Common.cpp)

set(backend_sources
	Common.cpp
	GrObject.cpp
	RenderGraph.cpp
	ShaderProgram.cpp
	Utils/StackGpuMemoryPool.cpp
	Common/Functions.cpp
	Utils/SegregatedListsGpuMemoryPool.cpp)

set(backend_headers
	AccelerationStructure.h
	Buffer.h
	CommandBuffer.h
	Common.h
	Fence.h
	Framebuffer.h
	GrManager.h
	GrObject.h
	OcclusionQuery.h
	RenderGraph.h
	RenderGraph.inl.h
	Sampler.h
	Shader.h
	ShaderProgram.h
	ShaderVariableDataType.def.h
	Texture.h
	TextureView.h
	TimestampQuery.h
	PipelineQuery.h
	GrUpscaler.h
	Utils/StackGpuMemoryPool.h
	Common/Functions.h
	Common/InstantiationMacros.def.h
	Common/Format.def.h
	Utils/SegregatedListsGpuMemoryPool.h)

if(VULKAN)
	file(GLOB_RECURSE vksources Vulkan/*.cpp)
	file(GLOB_RECURSE vkseaders Vulkan/*.h)

	set(backend_sources  ${backend_sources} ${vksources})
	set(backend_headers ${backend_headers} ${vkheaders})
elseif(DIRECTX)
	file(GLOB_RECURSE dxsources D3D/*.cpp)
	file(GLOB_RECURSE dxseaders D3D/*.h)

	set(backend_sources  ${backend_sources} ${dxsources})
	set(backend_headers ${backend_headers} ${dxheaders})

	set(extra_libs ${extra_libs} d3d12 dxgi)
endif()

# Have 2 libraries. The AnKiGrCommon is the bare minimum for the AnKiShaderCompiler to work. Don't have
# AnKiShaderCompiler depend on what AnKiGr depends on

add_library(AnKiGrCommon ${common_sources} ${common_headers})
target_compile_definitions(AnKiGrCommon PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiGrCommon AnKiUtil) # Only depend on Util

if(ANKI_DLSS)
	set(extra_libs ${extra_libs} AnKiNgx)
endif()

add_library(AnKiGr ${backend_sources} ${backend_headers})
target_compile_definitions(AnKiGr PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiGr AnKiCore AnKiSpirvCross AnKiGrCommon ${extra_libs})

# Copy DLSS files to Binaries
if(ANKI_DLSS)
	get_target_property(lib_type AnKiNgx TYPE)
	if("${lib_type}" STREQUAL "SHARED_LIBRARY")
		get_target_property(ANKINGX_DLL AnKiNgx IMPORTED_LOCATION)
		add_custom_command(
			TARGET AnKiGr
			COMMENT "Dependant DLLs copy ${ANKINGX_DLL} ${CMAKE_BINARY_DIR}/Binaries"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ANKINGX_DLL}" "${CMAKE_BINARY_DIR}/Binaries")
	else()
		get_property(__NGX_DLLS_LIST TARGET AnKiNgx PROPERTY ANKINGX_EXTRA_DLLS)
		foreach(dll_path ${__NGX_DLLS_LIST})
			file(GLOB dll_files "${dll_path}")
			foreach(dll_file ${dll_files})
				add_custom_command(
				TARGET AnKiGr
				COMMENT "NGX DLL copy ${dll_file} ${CMAKE_BINARY_DIR}/Binaries\n"
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll_file}" "${CMAKE_BINARY_DIR}/Binaries")
			endforeach()
		endforeach()
	endif()
endif()

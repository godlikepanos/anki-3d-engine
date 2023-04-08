set(common_sources
	Common.cpp
	GrObject.cpp
	RenderGraph.cpp
	ShaderProgram.cpp
	Utils/StackGpuMemoryPool.cpp
	Utils/Functions.cpp
	Utils/SegregatedListsGpuMemoryPool.cpp)

set(common_headers
	AccelerationStructure.h
	Buffer.h
	CommandBuffer.h
	Common.h
	ConfigVars.defs.h
	Fence.h
	Format.defs.h
	Framebuffer.h
	GrManager.h
	GrObject.h
	OcclusionQuery.h
	RenderGraph.h
	RenderGraph.inl.h
	Sampler.h
	Shader.h
	ShaderProgram.h
	ShaderVariableDataType.defs.h
	Texture.h
	TextureView.h
	TimestampQuery.h
	GrUpscaler.h
	Utils/StackGpuMemoryPool.h
	Utils/Functions.h
	Utils/InstantiationMacros.h
	Utils/SegregatedListsGpuMemoryPool.h)

if(VULKAN)
	set(backend_sources
		Vulkan/AccelerationStructure.cpp
		Vulkan/AccelerationStructureImpl.cpp
		Vulkan/Buffer.cpp
		Vulkan/BufferImpl.cpp
		Vulkan/CommandBuffer.cpp
		Vulkan/CommandBufferFactory.cpp
		Vulkan/CommandBufferImpl.cpp
		Vulkan/Common.cpp
		Vulkan/DescriptorSet.cpp
		Vulkan/Fence.cpp
		Vulkan/FenceFactory.cpp
		Vulkan/Framebuffer.cpp
		Vulkan/FramebufferImpl.cpp
		Vulkan/GpuMemoryManager.cpp
		Vulkan/GrManager.cpp
		Vulkan/GrManagerImpl.cpp
		Vulkan/OcclusionQuery.cpp
		Vulkan/OcclusionQueryImpl.cpp
		Vulkan/PipelineCache.cpp
		Vulkan/Pipeline.cpp
		Vulkan/PipelineLayout.cpp
		Vulkan/QueryFactory.cpp
		Vulkan/Sampler.cpp
		Vulkan/SamplerFactory.cpp
		Vulkan/SamplerImpl.cpp
		Vulkan/Shader.cpp
		Vulkan/ShaderImpl.cpp
		Vulkan/ShaderProgram.cpp
		Vulkan/ShaderProgramImpl.cpp
		Vulkan/SwapchainFactory.cpp
		Vulkan/Texture.cpp
		Vulkan/TextureImpl.cpp
		Vulkan/TextureView.cpp
		Vulkan/TextureViewImpl.cpp
		Vulkan/TimestampQuery.cpp
		Vulkan/TimestampQueryImpl.cpp
		Vulkan/FrameGarbageCollector.cpp
		Vulkan/GrUpscaler.cpp
		Vulkan/GrUpscalerImpl.cpp)

	set(backend_headers
		Vulkan/AccelerationStructureImpl.h
		Vulkan/BufferImpl.h
		Vulkan/CommandBufferFactory.h
		Vulkan/CommandBufferImpl.h
		Vulkan/CommandBufferImpl.inl.h
		Vulkan/Common.h
		Vulkan/DeferredBarrierFactory.h
		Vulkan/DescriptorSet.h
		Vulkan/FenceFactory.h
		Vulkan/FenceFactory.inl.h
		Vulkan/FenceImpl.h
		Vulkan/FramebufferImpl.h
		Vulkan/FrameGarbageCollector.h
		Vulkan/GpuMemoryManager.h
		Vulkan/GrManagerImpl.h
		Vulkan/MicroObjectRecycler.h
		Vulkan/MicroObjectRecycler.inl.h
		Vulkan/OcclusionQueryImpl.h
		Vulkan/Pipeline.h
		Vulkan/PipelineCache.h
		Vulkan/PipelineLayout.h
		Vulkan/QueryFactory.h
		Vulkan/SamplerFactory.h
		Vulkan/SamplerImpl.h
		Vulkan/SemaphoreFactory.h
		Vulkan/SemaphoreFactory.inl.h
		Vulkan/ShaderImpl.h
		Vulkan/ShaderProgramImpl.h
		Vulkan/SwapchainFactory.h
		Vulkan/TextureImpl.h
		Vulkan/TextureViewImpl.h
		Vulkan/TimestampQueryImpl.h
		Vulkan/GrUpscalerImpl.h)

	if(ANKI_HEADLESS)
		set(backend_sources ${backend_sources} "Vulkan/GrManagerImplHeadless.cpp")
	elseif(SDL)
		set(backend_sources ${backend_sources} "Vulkan/GrManagerImplSdl.cpp")
	elseif(ANDROID)
		set(backend_sources ${backend_sources} "Vulkan/GrManagerImplAndroid.cpp")
	endif()
endif()

# Have 2 libraries. The AnKiGrCommon is the bare minimum for the AnKiShaderCompiler to work. Don't have
# AnKiShaderCompiler depend on what AnKiGr depends on

add_library(AnKiGrCommon ${common_sources} ${common_headers})
target_compile_definitions(AnKiGrCommon PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiGrCommon AnKiUtil) # Only depend on Util

if(ANKI_DLSS)
	set(extra_libs AnKiNgx)
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

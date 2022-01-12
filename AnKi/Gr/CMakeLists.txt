set(COMMON
	"Common.cpp"
	"GrObject.cpp"
	"RenderGraph.cpp"
	"ShaderProgram.cpp"
	"Utils/FrameGpuAllocator.cpp"
	"Utils/Functions.cpp")

foreach(S ${COMMON})
	anki_add_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${S}")
endforeach()

if(VULKAN)
	set(VKCPP
		"Vulkan/AccelerationStructure.cpp"
		"Vulkan/AccelerationStructureImpl.cpp"
		"Vulkan/Buffer.cpp"
		"Vulkan/BufferImpl.cpp"
		"Vulkan/CommandBuffer.cpp"
		"Vulkan/CommandBufferFactory.cpp"
		"Vulkan/CommandBufferImpl.cpp"
		"Vulkan/Common.cpp"
		"Vulkan/DescriptorSet.cpp"
		"Vulkan/Fence.cpp"
		"Vulkan/FenceFactory.cpp"
		"Vulkan/Framebuffer.cpp"
		"Vulkan/FramebufferImpl.cpp"
		"Vulkan/GpuMemoryManager.cpp"
		"Vulkan/GrManager.cpp"
		"Vulkan/GrManagerImpl.cpp"
		"Vulkan/OcclusionQuery.cpp"
		"Vulkan/OcclusionQueryImpl.cpp"
		"Vulkan/PipelineCache.cpp"
		"Vulkan/Pipeline.cpp"
		"Vulkan/PipelineLayout.cpp"
		"Vulkan/QueryFactory.cpp"
		"Vulkan/Sampler.cpp"
		"Vulkan/SamplerFactory.cpp"
		"Vulkan/SamplerImpl.cpp"
		"Vulkan/Shader.cpp"
		"Vulkan/ShaderImpl.cpp"
		"Vulkan/ShaderProgram.cpp"
		"Vulkan/ShaderProgramImpl.cpp"
		"Vulkan/SwapchainFactory.cpp"
		"Vulkan/Texture.cpp"
		"Vulkan/TextureImpl.cpp"
		"Vulkan/TextureView.cpp"
		"Vulkan/TextureViewImpl.cpp"
		"Vulkan/TimestampQuery.cpp"
		"Vulkan/TimestampQueryImpl.cpp"
		"Vulkan/VulkanObject.cpp")

	if(ANKI_HEADLESS)
		set(VKCPP ${VKCPP} "Vulkan/GrManagerImplHeadless.cpp")
	elseif(SDL)
		set(VKCPP ${VKCPP} "Vulkan/GrManagerImplSdl.cpp")
	elseif(ANDROID)
		set(VKCPP ${VKCPP} "Vulkan/GrManagerImplAndroid.cpp")
	endif()

	foreach(S ${VKCPP})
		anki_add_source_files("${CMAKE_CURRENT_SOURCE_DIR}/${S}")
	endforeach()
endif()

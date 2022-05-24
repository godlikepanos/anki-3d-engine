set(common_sources
	"Common.cpp"
	"GrObject.cpp"
	"RenderGraph.cpp"
	"ShaderProgram.cpp"
	"Enums.cpp"
	"Utils/FrameGpuAllocator.cpp"
	"Utils/Functions.cpp")

if(VULKAN)
	set(backend_sources
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
		"Vulkan/VulkanObject.cpp"
		"Vulkan/FrameGarbageCollector.cpp")

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

add_library(AnKiGrCommon ${common_sources})
target_compile_definitions(AnKiGrCommon PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiGrCommon AnKiUtil) # Only depend on Util

add_library(AnKiGr ${backend_sources})
target_compile_definitions(AnKiGr PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiGr AnKiCore AnKiSpirvCross AnKiGrCommon)

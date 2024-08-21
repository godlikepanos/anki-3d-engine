// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/Vulkan/VkDescriptor.h>
#include <AnKi/Gr/Vulkan/VkBuffer.h>

namespace anki {

// Forward
class GraphicsPipelineFactory;

/// @addtogroup vulkan
/// @{

/// Shader program implementation.
class ShaderProgramImpl final : public ShaderProgram
{
public:
	ShaderProgramImpl(CString name)
		: ShaderProgram(name)
	{
	}

	~ShaderProgramImpl();

	Error init(const ShaderProgramInitInfo& inf);

	Bool isGraphics() const
	{
		return !!(m_shaderTypes & ShaderTypeBit::kAllGraphics);
	}

	const VkPipelineShaderStageCreateInfo* getShaderCreateInfos(U32& count) const
	{
		ANKI_ASSERT(isGraphics());
		count = m_graphics.m_shaderCreateInfoCount;
		return &m_graphics.m_shaderCreateInfos[0];
	}

	const PipelineLayout2& getPipelineLayout() const
	{
		return *m_pplineLayout;
	}

	/// Only for graphics programs.
	GraphicsPipelineFactory& getGraphicsPipelineFactory()
	{
		ANKI_ASSERT(m_graphics.m_pplineFactory);
		return *m_graphics.m_pplineFactory;
	}

	VkPipeline getComputePipelineHandle() const
	{
		ANKI_ASSERT(m_compute.m_ppline);
		return m_compute.m_ppline;
	}

	VkPipeline getRayTracingPipelineHandle() const
	{
		ANKI_ASSERT(m_rt.m_ppline);
		return m_rt.m_ppline;
	}

	ShaderTypeBit getStages() const
	{
		ANKI_ASSERT(!!m_shaderTypes);
		return m_shaderTypes;
	}

	U32 getMissShaderCount() const
	{
		ANKI_ASSERT(m_rt.m_missShaderCount > 0);
		return m_rt.m_missShaderCount;
	}

	ConstWeakArray<U8> getShaderGroupHandlesInternal() const
	{
		ANKI_ASSERT(m_rt.m_allHandles.getSize() > 0);
		return m_rt.m_allHandles;
	}

	Buffer& getShaderGroupHandlesGpuBufferInternal() const
	{
		return *m_rt.m_allHandlesBuff;
	}

private:
	GrDynamicArray<ShaderPtr> m_shaders;

	PipelineLayout2* m_pplineLayout = nullptr;

	class
	{
	public:
		Array<VkPipelineShaderStageCreateInfo, U32(ShaderType::kPixel - ShaderType::kVertex) + 1> m_shaderCreateInfos = {};
		U32 m_shaderCreateInfoCount = 0;
		GraphicsPipelineFactory* m_pplineFactory = nullptr;
	} m_graphics;

	class
	{
	public:
		VkPipeline m_ppline = VK_NULL_HANDLE;
	} m_compute;

	class
	{
	public:
		VkPipeline m_ppline = VK_NULL_HANDLE;
		GrDynamicArray<U8> m_allHandles;
		U32 m_missShaderCount = 0;
		BufferPtr m_allHandlesBuff;
	} m_rt;

	void rewriteSpirv(ShaderReflectionDescriptorRelated& refl, GrDynamicArray<GrDynamicArray<U32>>& rewrittenSpirvs);
};
/// @}

} // end namespace anki

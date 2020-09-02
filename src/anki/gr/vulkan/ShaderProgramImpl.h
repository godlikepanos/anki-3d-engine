// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/ShaderProgram.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/PipelineLayout.h>

namespace anki
{

// Forward
class PipelineFactory;

/// @addtogroup vulkan
/// @{

class ShaderProgramReflectionInfo
{
public:
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colorAttachmentWritemask = {false};
	BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_attributeMask = {false};
	BitSet<MAX_DESCRIPTOR_SETS, U8> m_descriptorSetMask = {false};
	Array<BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET, U8>, MAX_DESCRIPTOR_SETS> m_activeBindingMask = {{{false}, {false}}};
	U32 m_pushConstantsSize = 0;
};

/// Shader program implementation.
class ShaderProgramImpl final : public ShaderProgram, public VulkanObject<ShaderProgram, ShaderProgramImpl>
{
public:
	ShaderProgramImpl(GrManager* manager, CString name)
		: ShaderProgram(manager, name)
	{
	}

	~ShaderProgramImpl();

	Error init(const ShaderProgramInitInfo& inf);

	Bool isGraphics() const
	{
		return !!(m_stages & ShaderTypeBit::ALL_GRAPHICS);
	}

	const VkPipelineShaderStageCreateInfo* getShaderCreateInfos(U32& count) const
	{
		ANKI_ASSERT(isGraphics());
		count = m_graphics.m_shaderCreateInfoCount;
		return &m_graphics.m_shaderCreateInfos[0];
	}

	const PipelineLayout& getPipelineLayout() const
	{
		return m_pplineLayout;
	}

	const DescriptorSetLayout& getDescriptorSetLayout(U32 set) const
	{
		ANKI_ASSERT(m_descriptorSetLayouts[set].isCreated());
		return m_descriptorSetLayouts[set];
	}

	const ShaderProgramReflectionInfo& getReflectionInfo() const
	{
		return m_refl;
	}

	/// Only for graphics programs.
	PipelineFactory& getPipelineFactory()
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
		ANKI_ASSERT(!!m_stages);
		return m_stages;
	}

	U32 getMissShaderCount() const
	{
		ANKI_ASSERT(m_rt.m_missShaderCount > 0);
		return m_rt.m_missShaderCount;
	}

	ConstWeakArray<U8> getShaderGroupHandles() const
	{
		ANKI_ASSERT(m_rt.m_allHandles.getSize() > 0);
		return m_rt.m_allHandles;
	}

private:
	DynamicArray<ShaderPtr> m_shaders;
	ShaderTypeBit m_stages = ShaderTypeBit::NONE;

	PipelineLayout m_pplineLayout = {};
	Array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> m_descriptorSetLayouts;

	ShaderProgramReflectionInfo m_refl;

	class
	{
	public:
		Array<VkPipelineShaderStageCreateInfo, U32(ShaderType::FRAGMENT - ShaderType::VERTEX) + 1> m_shaderCreateInfos;
		U32 m_shaderCreateInfoCount = 0;
		PipelineFactory* m_pplineFactory = nullptr;
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
		DynamicArray<U8> m_allHandles;
		U32 m_missShaderCount = 0;
	} m_rt;
};
/// @}

} // end namespace anki

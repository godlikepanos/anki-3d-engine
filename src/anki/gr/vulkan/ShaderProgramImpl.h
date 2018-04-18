// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
		return m_pplineFactory != nullptr;
	}

	const VkPipelineShaderStageCreateInfo* getShaderCreateInfos(U32& count) const
	{
		ANKI_ASSERT(isGraphics());
		count = m_shaderCreateInfoCount;
		return &m_shaderCreateInfos[0];
	}

	const PipelineLayout& getPipelineLayout() const
	{
		return m_pplineLayout;
	}

	const DescriptorSetLayout& getDescriptorSetLayout(U set) const
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
		ANKI_ASSERT(m_pplineFactory);
		return *m_pplineFactory;
	}

	VkPipeline getComputePipelineHandle() const
	{
		ANKI_ASSERT(m_computePpline);
		return m_computePpline;
	}

	ShaderTypeBit getStages() const
	{
		ANKI_ASSERT(!!m_stages);
		return m_stages;
	}

private:
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders;
	ShaderTypeBit m_stages = ShaderTypeBit::NONE;

	Array<VkPipelineShaderStageCreateInfo, U(ShaderType::COUNT) - 1> m_shaderCreateInfos;
	U32 m_shaderCreateInfoCount = 0;

	PipelineLayout m_pplineLayout = {};
	Array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> m_descriptorSetLayouts;

	ShaderProgramReflectionInfo m_refl;

	PipelineFactory* m_pplineFactory = nullptr; ///< Only for graphics programs.

	VkPipeline m_computePpline = VK_NULL_HANDLE;
};
/// @}

} // end namespace anki

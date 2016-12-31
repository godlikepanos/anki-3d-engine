// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/PipelineLayout.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Shader program implementation.
class ShaderProgramImpl : public VulkanObject
{
public:
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders;

	Array<VkPipelineShaderStageCreateInfo, U(ShaderType::COUNT) - 1> m_shaderCreateInfos;
	U32 m_shaderCreateInfoCount = 0;
	PipelineLayout m_pplineLayout;

	Array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> m_descriptorSetLayouts;
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colorAttachmentWritemask = {false};
	BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_attributeMask = {false};

	ShaderProgramImpl(GrManager* manager);
	~ShaderProgramImpl();

	Error init(const Array<ShaderPtr, U(ShaderType::COUNT)>& shaders);
};
/// @}

} // end namespace anki

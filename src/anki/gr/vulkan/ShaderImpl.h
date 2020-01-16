// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Shader.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Shader vulkan implementation.
class ShaderImpl final : public Shader, public VulkanObject<Shader, ShaderImpl>
{
public:
	VkShaderModule m_handle = VK_NULL_HANDLE;

	Array<DynamicArray<DescriptorBinding>, MAX_DESCRIPTOR_SETS> m_bindings;
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colorAttachmentWritemask = {false};
	BitSet<MAX_VERTEX_ATTRIBUTES, U8> m_attributeMask = {false};
	BitSet<MAX_DESCRIPTOR_SETS, U8> m_descriptorSetMask = {false};
	Array<BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET, U8>, MAX_DESCRIPTOR_SETS> m_activeBindingMask = {{{false}, {false}}};
	U32 m_pushConstantsSize = 0;

	ShaderImpl(GrManager* manager, CString name)
		: Shader(manager, name)
	{
	}

	~ShaderImpl();

	ANKI_USE_RESULT Error init(const ShaderInitInfo& init);

	const VkSpecializationInfo* getSpecConstInfo() const
	{
		return (m_specConstInfo.mapEntryCount) ? &m_specConstInfo : nullptr;
	}

private:
	VkSpecializationInfo m_specConstInfo = {};

	class SpecConstsVector; ///< Wrap this into a class to avoid forward declarations of std::vector and spirv_cross.

	void doReflection(ConstWeakArray<U8> spirv, SpecConstsVector& specConstIds);
};
/// @}

} // end namespace anki

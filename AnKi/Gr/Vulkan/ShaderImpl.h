// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/DescriptorSet.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Shader vulkan implementation.
class ShaderImpl final : public Shader, public VulkanObject<Shader, ShaderImpl>
{
public:
	VkShaderModule m_handle = VK_NULL_HANDLE;

	Array<DynamicArray<DescriptorBinding>, kMaxDescriptorSets> m_bindings;
	BitSet<kMaxColorRenderTargets, U8> m_colorAttachmentWritemask = {false};
	BitSet<kMaxVertexAttributes, U8> m_attributeMask = {false};
	BitSet<kMaxDescriptorSets, U8> m_descriptorSetMask = {false};
	Array<BitSet<kMaxBindingsPerDescriptorSet, U8>, kMaxDescriptorSets> m_activeBindingMask = {
		{{false}, {false}, {false}}};
	U32 m_pushConstantsSize = 0;

	ShaderImpl(GrManager* manager, CString name)
		: Shader(manager, name)
	{
	}

	~ShaderImpl();

	Error init(const ShaderInitInfo& init);

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

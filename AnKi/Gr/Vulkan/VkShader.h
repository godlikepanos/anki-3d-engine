// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/Vulkan/VkDescriptorSetFactory.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Shader vulkan implementation.
class ShaderImpl final : public Shader
{
public:
	VkShaderModule m_handle = VK_NULL_HANDLE;
	ShaderReflection m_reflection;

	ShaderImpl(CString name)
		: Shader(name)
	{
	}

	~ShaderImpl();

	Error init(const ShaderInitInfo& init);
};
/// @}

} // end namespace anki

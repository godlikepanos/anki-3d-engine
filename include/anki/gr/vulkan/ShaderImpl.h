// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/util/String.h>
#include <vector>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Shader vulkan implementation.
class ShaderImpl : public VulkanObject
{
public:
	VkShaderModule m_handle = VK_NULL_HANDLE;
	ShaderType m_shaderType = ShaderType::COUNT;

	ShaderImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~ShaderImpl();

	ANKI_USE_RESULT Error init(ShaderType shaderType, const CString& source);

private:
	/// Generate SPIRV from GLSL.
	ANKI_USE_RESULT Error genSpirv(
		const CString& source, std::vector<unsigned int>& spirv);
};
/// @}

} // end namespace anki

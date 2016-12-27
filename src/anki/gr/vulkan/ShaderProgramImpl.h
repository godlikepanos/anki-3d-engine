// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/DescriptorSet.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Shader program implementation.
class ShaderProgramImpl : public VulkanObject
{
public:
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders;

	ShaderProgramImpl(GrManager* manager);
	~ShaderProgramImpl();

	Error init(const Array<ShaderPtr, U(ShaderType::COUNT)>& shaders);
};
/// @}

} // end namespace anki

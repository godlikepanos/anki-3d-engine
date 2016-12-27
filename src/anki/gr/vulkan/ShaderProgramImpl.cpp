// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/gr/Shader.h>
#include <anki/gr/vulkan/ShaderImpl.h>

namespace anki
{

ShaderProgramImpl::ShaderProgramImpl(GrManager* manager)
	: VulkanObject(manager)
{
}

ShaderProgramImpl::~ShaderProgramImpl()
{
}

Error ShaderProgramImpl::init(const Array<ShaderPtr, U(ShaderType::COUNT)>& shaders)
{
	m_shaders = shaders;

	return ErrorCode::NONE;
}

} // end namespace anki

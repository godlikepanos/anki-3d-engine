// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/Vulkan/ShaderProgramImpl.h>
#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

ShaderProgram* ShaderProgram::newInstance(const ShaderProgramInitInfo& init)
{
	ShaderProgramImpl* impl = anki::newInstance<ShaderProgramImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

ConstWeakArray<U8> ShaderProgram::getShaderGroupHandles() const
{
	return static_cast<const ShaderProgramImpl&>(*this).getShaderGroupHandles();
}

} // end namespace anki

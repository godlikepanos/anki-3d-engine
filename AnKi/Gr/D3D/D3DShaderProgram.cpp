// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DShaderProgram.h>

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
	ANKI_ASSERT(!"TODO");
	return ConstWeakArray<U8>();
}

Buffer& ShaderProgram::getShaderGroupHandlesGpuBuffer() const
{
	ANKI_ASSERT(!"TODO");
	void* ptr = nullptr;
	return *reinterpret_cast<Buffer*>(ptr);
}

ShaderProgramImpl::~ShaderProgramImpl()
{
}

Error ShaderProgramImpl::init(const ShaderProgramInitInfo& inf)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki

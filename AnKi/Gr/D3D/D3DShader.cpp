// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DShader.h>

namespace anki {

Shader* Shader::newInstance(const ShaderInitInfo& init)
{
	ShaderImpl* impl = anki::newInstance<ShaderImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

ShaderImpl::~ShaderImpl()
{
}

Error ShaderImpl::init(const ShaderInitInfo& inf)
{
	m_shaderType = inf.m_shaderType;
	m_shaderBinarySize = U32(inf.m_binary.getSizeInBytes());
	m_hasDiscard = inf.m_reflection.m_fragment.m_discards;
	m_reflection = inf.m_reflection;
	m_reflection.validate();

	m_binary.resize(m_shaderBinarySize);
	memcpy(m_binary.getBegin(), inf.m_binary.getBegin(), inf.m_binary.getSizeInBytes());

	return Error::kNone;
}

} // end namespace anki

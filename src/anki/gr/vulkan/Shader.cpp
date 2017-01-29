// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Shader.h>
#include <anki/gr/vulkan/ShaderImpl.h>

namespace anki
{

Shader::Shader(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

Shader::~Shader()
{
}

void Shader::init(ShaderType shaderType, const CString& source)
{
	ANKI_ASSERT(!source.isEmpty());

	m_impl.reset(getAllocator().newInstance<ShaderImpl>(&getManager()));

	if(m_impl->init(shaderType, source))
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

} // end namespace anki

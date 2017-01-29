// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderProgram.h>
#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/gr/Shader.h>

namespace anki
{

ShaderProgram::ShaderProgram(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::init(ShaderPtr vert, ShaderPtr frag)
{
	ANKI_ASSERT(vert && frag);

	Array<ShaderPtr, U(ShaderType::COUNT)> shaders = {};
	shaders[ShaderType::VERTEX] = vert;
	shaders[ShaderType::FRAGMENT] = frag;

	m_impl.reset(getAllocator().newInstance<ShaderProgramImpl>(&getManager()));

	if(m_impl->init(shaders))
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

void ShaderProgram::init(ShaderPtr comp)
{
	ANKI_ASSERT(comp);

	Array<ShaderPtr, U(ShaderType::COUNT)> shaders = {};
	shaders[ShaderType::COMPUTE] = comp;

	m_impl.reset(getAllocator().newInstance<ShaderProgramImpl>(&getManager()));

	if(m_impl->init(shaders))
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

void ShaderProgram::init(ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag)
{
	ANKI_ASSERT(vert && frag);

	Array<ShaderPtr, U(ShaderType::COUNT)> shaders = {};
	shaders[ShaderType::VERTEX] = vert;
	shaders[ShaderType::TESSELLATION_CONTROL] = tessc;
	shaders[ShaderType::TESSELLATION_EVALUATION] = tesse;
	shaders[ShaderType::GEOMETRY] = geom;
	shaders[ShaderType::FRAGMENT] = frag;

	m_impl.reset(getAllocator().newInstance<ShaderProgramImpl>(&getManager()));

	if(m_impl->init(shaders))
	{
		ANKI_VK_LOGF("Cannot recover");
	}
}

} // end namespace anki

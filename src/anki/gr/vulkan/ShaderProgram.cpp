// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderProgram.h>
#include <anki/gr/vulkan/ShaderProgramImpl.h>

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
	ANKI_ASSERT(!"TODO");
}

void ShaderProgram::init(ShaderPtr comp)
{
	ANKI_ASSERT(!"TODO");
}

void ShaderProgram::init(ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag)
{
	ANKI_ASSERT(!"TODO");
}

} // end namespace anki

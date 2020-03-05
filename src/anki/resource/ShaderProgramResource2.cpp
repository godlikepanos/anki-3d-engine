// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramResource2.h>

namespace anki
{

ShaderProgramResource2::ShaderProgramResource2(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ShaderProgramResource2::~ShaderProgramResource2()
{
}

Error ShaderProgramResource2::load(const ResourceFilename& filename, Bool async)
{
	return Error::NONE;
}

} // end namespace anki

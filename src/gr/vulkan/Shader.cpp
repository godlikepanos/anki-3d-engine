// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Shader.h>
#include <anki/gr/vulkan/ShaderImpl.h>

namespace anki
{

//==============================================================================
Shader::Shader(GrManager* manager, U64 hash)
    : GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
Shader::~Shader()
{
}

//==============================================================================
void Shader::init(ShaderType shaderType, const void* source, PtrSize sourceSize)
{
}

} // end namespace anki
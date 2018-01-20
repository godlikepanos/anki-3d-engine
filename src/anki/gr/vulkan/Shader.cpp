// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Shader.h>
#include <anki/gr/vulkan/ShaderImpl.h>
#include <anki/gr/GrManager.h>

namespace anki
{

Shader* Shader::newInstance(GrManager* manager, const ShaderInitInfo& init)
{
	return ShaderImpl::newInstanceHelper(manager, init);
}

} // end namespace anki

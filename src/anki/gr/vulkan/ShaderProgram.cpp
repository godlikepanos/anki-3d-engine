// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderProgram.h>
#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/gr/Shader.h>
#include <anki/gr/GrManager.h>

namespace anki
{

ShaderProgram* ShaderProgram::newInstance(GrManager* manager, const ShaderProgramInitInfo& init)
{
	ShaderProgramImpl* impl = manager->getAllocator().newInstance<ShaderProgramImpl>(manager, init.getName());
	Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
	}
	return impl;
}

} // end namespace anki

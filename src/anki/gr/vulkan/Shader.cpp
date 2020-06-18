// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	ShaderImpl* impl = manager->getAllocator().newInstance<ShaderImpl>(manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki

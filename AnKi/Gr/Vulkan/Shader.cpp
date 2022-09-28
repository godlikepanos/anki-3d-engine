// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/Vulkan/ShaderImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

Shader* Shader::newInstance(GrManager* manager, const ShaderInitInfo& init)
{
	ShaderImpl* impl = anki::newInstance<ShaderImpl>(manager->getMemoryPool(), manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(manager->getMemoryPool(), impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki

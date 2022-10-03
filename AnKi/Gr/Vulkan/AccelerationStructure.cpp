// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Gr/Vulkan/AccelerationStructureImpl.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

AccelerationStructure* AccelerationStructure::newInstance(GrManager* manager, const AccelerationStructureInitInfo& init)
{
	AccelerationStructureImpl* impl =
		anki::newInstance<AccelerationStructureImpl>(manager->getMemoryPool(), manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(manager->getMemoryPool(), impl);
		impl = nullptr;
	}
	return impl;
}

} // end namespace anki

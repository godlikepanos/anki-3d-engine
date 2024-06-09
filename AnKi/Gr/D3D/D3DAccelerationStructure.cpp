// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DAccelerationStructure.h>

namespace anki {

AccelerationStructure* AccelerationStructure::newInstance(const AccelerationStructureInitInfo& init)
{
	AccelerationStructureImpl* impl = anki::newInstance<AccelerationStructureImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

U64 AccelerationStructure::getGpuAddress() const
{
	ANKI_ASSERT(!"TODO");
	return 0;
}

AccelerationStructureImpl::~AccelerationStructureImpl()
{
}

} // end namespace anki

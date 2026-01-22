// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

namespace anki {

MicroFencePtr FenceFactory::newInstance(CString name)
{
	MicroFence* fence = m_recycler.findToReuse();

	if(fence == nullptr)
	{
		fence = newInstance<MicroFence>(GrMemoryPool::getSingleton());
	}
	else
	{
		fence->reset();
	}

	ANKI_ASSERT(fence->getRefcount() == 0);

	getGrManagerImpl().trySetVulkanHandleName(name, VK_OBJECT_TYPE_FENCE, fence->getHandle());

	return MicroFencePtr(fence);
}

} // end namespace anki

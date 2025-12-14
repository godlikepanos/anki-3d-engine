// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkFenceFactory.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

namespace anki {

void MicroFenceImpl::setName(CString name) const
{
	ANKI_ASSERT(m_handle);
	getGrManagerImpl().trySetVulkanHandleName(name, VK_OBJECT_TYPE_FENCE, m_handle);
}

} // end namespace anki

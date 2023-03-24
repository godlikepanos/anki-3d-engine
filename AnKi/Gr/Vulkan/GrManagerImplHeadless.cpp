// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Window/NativeWindow.h>

namespace anki {

Error GrManagerImpl::initSurface()
{
	VkHeadlessSurfaceCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;

	ANKI_VK_CHECK(vkCreateHeadlessSurfaceEXT(m_instance, &createInfo, nullptr, &m_surface));

	m_nativeWindowWidth = NativeWindow::getSingleton().getWidth();
	m_nativeWindowHeight = NativeWindow::getSingleton().getHeight();

	return Error::kNone;
}

} // end namespace anki

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Core/NativeWindowAndroid.h>

namespace anki {

Error GrManagerImpl::initSurface(const GrManagerInitInfo& init)
{
	VkAndroidSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.window = static_cast<NativeWindowAndroid*>(init.m_window)->m_nativeWindow;

	ANKI_VK_CHECK(vkCreateAndroidSurfaceKHR(m_instance, &createInfo, nullptr, &m_surface));

	return Error::kNone;
}

} // end namespace anki

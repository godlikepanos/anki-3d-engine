// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/NativeWindowSdl.h>
#include <SDL_syswm.h>
#if ANKI_OS == ANKI_OS_LINUX
#include <X11/Xlib-xcb.h>
#else
#error TODO
#endif

namespace anki
{

Error GrManagerImpl::initSurface(const GrManagerInitInfo& init)
{
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);
	if(!SDL_GetWindowWMInfo(init.m_window->getNative().m_window, &wminfo))
	{
		ANKI_LOGE("SDL_GetWindowWMInfo() failed");
		return ErrorCode::NONE;
	}

#if ANKI_OS == ANKI_OS_LINUX
	VkXcbSurfaceCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	ci.connection = XGetXCBConnection(wminfo.info.x11.display);
	ci.window = wminfo.info.x11.window;

	ANKI_VK_CHECK(vkCreateXcbSurfaceKHR(m_instance, &ci, nullptr, &m_surface));
#else
#error TODO
#endif

	return ErrorCode::NONE;
}

} // end namespace anki
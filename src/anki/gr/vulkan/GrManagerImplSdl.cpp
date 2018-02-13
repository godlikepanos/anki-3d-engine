// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/NativeWindowSdl.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>
#if ANKI_OS == ANKI_OS_LINUX
#	include <X11/Xlib-xcb.h>
#elif ANKI_OS == ANKI_OS_WINDOWS
#	include <Winuser.h>
#else
#	error TODO
#endif

namespace anki
{

Error GrManagerImpl::initSurface(const GrManagerInitInfo& init)
{
#if ANKI_OS == ANKI_OS_LINUX
	if(!SDL_Vulkan_CreateSurface(init.m_window->getNative().m_window, m_instance, &m_surface))
	{
		ANKI_VK_LOGE("SDL_Vulkan_CreateSurface() failed: %s", SDL_GetError());
		return Error::FUNCTION_FAILED;
	}
#elif ANKI_OS == ANKI_OS_WINDOWS
	SDL_SysWMinfo wminfo;
	SDL_VERSION(&wminfo.version);
	if(!SDL_GetWindowWMInfo(init.m_window->getNative().m_window, &wminfo))
	{
		ANKI_VK_LOGE("SDL_GetWindowWMInfo() failed");
		return Error::NONE;
	}

	Array<TCHAR, 512> className;
	GetClassName(wminfo.info.win.window, &className[0], className.getSize());

	WNDCLASS wce = {};
	GetClassInfo(GetModuleHandle(NULL), &className[0], &wce);

	VkWin32SurfaceCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	ci.hinstance = wce.hInstance;
	ci.hwnd = wminfo.info.win.window;

	ANKI_VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &ci, nullptr, &m_surface));
#else
#	error TODO
#endif

	return Error::NONE;
}

} // end namespace anki

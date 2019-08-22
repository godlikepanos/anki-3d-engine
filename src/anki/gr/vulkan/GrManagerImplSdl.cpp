// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/core/NativeWindow.h>
#include <anki/core/NativeWindowSdl.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>
#if ANKI_OS_LINUX
#	include <X11/Xlib-xcb.h>
#elif ANKI_OS_WINDOWS
#	include <Winuser.h>
#	include <anki/util/CleanupWindows.h>
#else
#	error Not supported
#endif

namespace anki
{

Error GrManagerImpl::initSurface(const GrManagerInitInfo& init)
{
	if(!SDL_Vulkan_CreateSurface(init.m_window->getNative().m_window, m_instance, &m_surface))
	{
		ANKI_VK_LOGE("SDL_Vulkan_CreateSurface() failed: %s", SDL_GetError());
		return Error::FUNCTION_FAILED;
	}

	return Error::NONE;
}

} // end namespace anki

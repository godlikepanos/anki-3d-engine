// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Window/NativeWindow.h>
#include <AnKi/Window/NativeWindowSdl.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

// Because some idiot includes Windows.h
#if defined(ERROR)
#	undef ERROR
#endif

namespace anki {

Error GrManagerImpl::initSurface()
{
	if(!SDL_Vulkan_CreateSurface(static_cast<NativeWindowSdl&>(NativeWindow::getSingleton()).m_sdlWindow, m_instance,
								 &m_surface))
	{
		ANKI_VK_LOGE("SDL_Vulkan_CreateSurface() failed: %s", SDL_GetError());
		return Error::kFunctionFailed;
	}

	return Error::kNone;
}

} // end namespace anki

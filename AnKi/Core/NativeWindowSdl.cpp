// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/NativeWindowSdl.h>
#include <AnKi/Util/Logger.h>
#if ANKI_GR_BACKEND_VULKAN
#	include <SDL_vulkan.h>
#endif

namespace anki {

Error NativeWindow::newInstance(const NativeWindowInitInfo& initInfo, NativeWindow*& nativeWindow)
{
	HeapAllocator<U8> alloc(initInfo.m_allocCallback, initInfo.m_allocCallbackUserData, "NativeWindow");
	NativeWindowSdl* sdlwin = alloc.newInstance<NativeWindowSdl>();

	sdlwin->m_alloc = alloc;

	const Error err = sdlwin->init(initInfo);
	if(err)
	{
		alloc.deleteInstance(sdlwin);
		nativeWindow = nullptr;
		return err;
	}
	else
	{
		nativeWindow = sdlwin;
		return Error::NONE;
	}
}

void NativeWindow::deleteInstance(NativeWindow* window)
{
	if(window)
	{
		NativeWindowSdl* self = static_cast<NativeWindowSdl*>(window);
		HeapAllocator<U8> alloc = self->m_alloc;
		alloc.deleteInstance(self);
	}
}

void NativeWindow::setWindowTitle(CString title)
{
	NativeWindowSdl* self = static_cast<NativeWindowSdl*>(this);
	SDL_SetWindowTitle(self->m_window, title.cstr());
}

NativeWindowSdl::~NativeWindowSdl()
{
	if(m_window)
	{
		SDL_DestroyWindow(m_window);
	}

	SDL_QuitSubSystem(INIT_SUBSYSTEMS);
	SDL_Quit();
}

Error NativeWindowSdl::init(const NativeWindowInitInfo& init)
{
	if(SDL_Init(INIT_SUBSYSTEMS) != 0)
	{
		ANKI_CORE_LOGE("SDL_Init() failed: %s", SDL_GetError());
		return Error::FUNCTION_FAILED;
	}

#if ANKI_GR_BACKEND_VULKAN
	if(SDL_Vulkan_LoadLibrary(nullptr))
	{
		ANKI_CORE_LOGE("SDL_Vulkan_LoadLibrary() failed: %s", SDL_GetError());
		return Error::FUNCTION_FAILED;
	}
#endif

	//
	// Set GL attributes
	//
	ANKI_CORE_LOGI("Creating SDL window. SDL version %u.%u", SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

	//
	// Create window
	//
	U32 flags = 0;

#if ANKI_GR_BACKEND_GL
	flags |= SDL_WINDOW_OPENGL;
#elif ANKI_GR_BACKEND_VULKAN
	flags |= SDL_WINDOW_VULKAN;
#endif

	SDL_SetHint(SDL_HINT_ALLOW_TOPMOST, "0");
	if(init.m_fullscreenDesktopRez)
	{
#if ANKI_OS_WINDOWS
		flags |= SDL_WINDOW_FULLSCREEN;

		if(init.m_exclusiveFullscreen)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
#elif ANKI_OS_LINUX
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
#	error See file
#endif

		// Alter the window size
		SDL_DisplayMode mode;
		if(SDL_GetDesktopDisplayMode(0, &mode))
		{
			ANKI_CORE_LOGE("SDL_GetDesktopDisplayMode() failed: %s", SDL_GetError());
			return Error::FUNCTION_FAILED;
		}

		m_width = mode.w;
		m_height = mode.h;
	}
	else
	{
		m_width = init.m_width;
		m_height = init.m_height;
	}

	m_window =
		SDL_CreateWindow(&init.m_title[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, flags);

	if(m_window == nullptr)
	{
		ANKI_CORE_LOGE("SDL_CreateWindow() failed");
		return Error::FUNCTION_FAILED;
	}

	// Final check
	{
		int w, h;
		SDL_GetWindowSize(m_window, &w, &h);
		ANKI_ASSERT(m_width == U32(w) && m_height == U32(h));
	}

	ANKI_CORE_LOGI("SDL window created");
	return Error::NONE;
}

} // end namespace anki

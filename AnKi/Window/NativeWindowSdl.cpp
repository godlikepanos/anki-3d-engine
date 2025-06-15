// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Window/NativeWindowSdl.h>
#include <AnKi/Util/Logger.h>
#if ANKI_GR_BACKEND_VULKAN
#	include <SDL3/SDL_vulkan.h>
#endif

namespace anki {

template<>
template<>
NativeWindow& MakeSingletonPtr<NativeWindow>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new NativeWindowSdl();
#if ANKI_ASSERTIONS_ENABLED
	++g_singletonsAllocated;
#endif
	return *m_global;
}

template<>
void MakeSingletonPtr<NativeWindow>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<NativeWindowSdl*>(m_global);
		m_global = nullptr;
#if ANKI_ASSERTIONS_ENABLED
		--g_singletonsAllocated;
#endif
	}
}

Error NativeWindow::init(const NativeWindowInitInfo& inf)
{
	return static_cast<NativeWindowSdl*>(this)->initSdl(inf);
}

void NativeWindow::setWindowTitle(CString title)
{
	NativeWindowSdl* self = static_cast<NativeWindowSdl*>(this);
	SDL_SetWindowTitle(self->m_sdlWindow, title.cstr());
}

NativeWindowSdl::~NativeWindowSdl()
{
	if(m_sdlWindow)
	{
		SDL_DestroyWindow(m_sdlWindow);
	}

	SDL_QuitSubSystem(kInitSubsystems);
	SDL_Quit();
}

Error NativeWindowSdl::initSdl(const NativeWindowInitInfo& init)
{
#if ANKI_OS_WINDOWS
	// Tell windows that the app will handle scaling. Otherwise SDL_GetDesktopDisplayMode will return a resolution that has the scaling applied
	if(!SetProcessDPIAware())
	{
		ANKI_WIND_LOGE("SetProcessDPIAware() failed");
	}
#endif

	if(!SDL_Init(kInitSubsystems))
	{
		ANKI_WIND_LOGE("SDL_Init() failed: %s", SDL_GetError());
		return Error::kFunctionFailed;
	}

#if ANKI_GR_BACKEND_VULKAN
	if(!SDL_Vulkan_LoadLibrary(nullptr))
	{
		ANKI_WIND_LOGE("SDL_Vulkan_LoadLibrary() failed: %s", SDL_GetError());
		return Error::kFunctionFailed;
	}
#endif

	//
	// Set GL attributes
	//
	ANKI_WIND_LOGI("Creating SDL window. SDL version %u.%u", SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

	//
	// Create window
	//
	U32 flags = SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_HIGH_PIXEL_DENSITY;

#if ANKI_GR_BACKEND_VULKAN
	flags |= SDL_WINDOW_VULKAN;
#endif

	if(init.m_fullscreenDesktopRez)
	{
		flags |= SDL_WINDOW_FULLSCREEN;

		SDL_DisplayID display = SDL_GetPrimaryDisplay();
		if(!display)
		{
			ANKI_WIND_LOGE("SDL_GetPrimaryDisplay() failed: %s", SDL_GetError());
			return Error::kFunctionFailed;
		}

		// Alter the window size
		const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
		if(!mode)
		{
			ANKI_WIND_LOGE("SDL_GetCurrentDisplayMode() failed: %s", SDL_GetError());
			return Error::kFunctionFailed;
		}

		m_width = mode->w;
		m_height = mode->h;
	}
	else
	{
		m_width = init.m_width;
		m_height = init.m_height;
	}

	m_sdlWindow = SDL_CreateWindow(&init.m_title[0], m_width, m_height, flags);

	if(m_sdlWindow == nullptr)
	{
		ANKI_WIND_LOGE("SDL_CreateWindow() failed");
		return Error::kFunctionFailed;
	}

	// Final check
	{
		int w, h;
		SDL_GetWindowSize(m_sdlWindow, &w, &h);
		ANKI_ASSERT(m_width == U32(w) && m_height == U32(h));
	}

	ANKI_WIND_LOGI("SDL window created");
	return Error::kNone;
}

} // end namespace anki

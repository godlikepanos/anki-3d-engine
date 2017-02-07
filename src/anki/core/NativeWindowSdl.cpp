// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/NativeWindowSdl.h>
#include <anki/util/Logger.h>

namespace anki
{

const U32 INIT_SUBSYSTEMS = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER;

Error NativeWindow::init(NativeWindowInitInfo& init, HeapAllocator<U8>& alloc)
{
	m_alloc = alloc;
	m_impl = m_alloc.newInstance<NativeWindowImpl>();

	if(SDL_Init(INIT_SUBSYSTEMS) != 0)
	{
		ANKI_CORE_LOGE("SDL_Init() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	//
	// Set GL attributes
	//
	ANKI_CORE_LOGI("Creating SDL window");

#if ANKI_GR_BACKEND == ANKI_GR_BACKEND_GL
	if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, init.m_rgbaBits[0])
		|| SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, init.m_rgbaBits[1])
		|| SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, init.m_rgbaBits[2])
		|| SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, init.m_rgbaBits[3])
		|| SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, init.m_depthBits)
		|| SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, init.m_doubleBuffer))
	{
		ANKI_CORE_LOGE("SDL_GL_SetAttribute() failed");
		return ErrorCode::FUNCTION_FAILED;
	}
#endif

	//
	// Create window
	//
	U32 flags = 0;

#if ANKI_GR_BACKEND == ANKI_GR_BACKEND_GL
	flags |= SDL_WINDOW_OPENGL;
#endif

	if(init.m_fullscreenDesktopRez)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	m_impl->m_window = SDL_CreateWindow(
		&init.m_title[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, init.m_width, init.m_height, flags);

	if(m_impl->m_window == nullptr)
	{
		ANKI_CORE_LOGE("SDL_CreateWindow() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Set the size after loading a fullscreen window
	if(init.m_fullscreenDesktopRez)
	{
		int w, h;
		SDL_GetWindowSize(m_impl->m_window, &w, &h);

		m_width = w;
		m_height = h;
	}
	else
	{
		m_width = init.m_width;
		m_height = init.m_height;
	}

	ANKI_CORE_LOGI("SDL window created");
	return ErrorCode::NONE;
}

void NativeWindow::destroy()
{
	if(m_impl != nullptr)
	{
		if(m_impl->m_window)
		{
			SDL_DestroyWindow(m_impl->m_window);
		}

		SDL_QuitSubSystem(INIT_SUBSYSTEMS);
		SDL_Quit();
	}

	m_alloc.deleteInstance(m_impl);
}

} // end namespace anki

// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/NativeWindowSdl.h>
#include <anki/util/Logger.h>
#include <GL/glew.h>

namespace anki
{

const U32 INIT_SUBSYSTEMS = SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS
	| SDL_INIT_GAMECONTROLLER;

//==============================================================================
Error NativeWindow::create(NativeWindowInitInfo& init, HeapAllocator<U8>& alloc)
{
	m_alloc = alloc;
	m_impl = m_alloc.newInstance<NativeWindowImpl>();

	if(SDL_Init(INIT_SUBSYSTEMS) != 0)
	{
		ANKI_LOGE("SDL_Init() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	//
	// Set GL attributes
	//
	ANKI_LOGI("Creating SDL window (OpenGL context to be requested %u.%u)...",
		init.m_majorVersion,
		init.m_minorVersion);

	if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, init.m_rgbaBits[0]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, init.m_rgbaBits[1]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, init.m_rgbaBits[2]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, init.m_rgbaBits[3]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, init.m_depthBits) != 0
		|| SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, init.m_doubleBuffer) != 0
		|| SDL_GL_SetAttribute(
			   SDL_GL_CONTEXT_MAJOR_VERSION, init.m_majorVersion)
			!= 0
		|| SDL_GL_SetAttribute(
			   SDL_GL_CONTEXT_MINOR_VERSION, init.m_minorVersion)
			!= 0
		|| SDL_GL_SetAttribute(
			   SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)
			!= 0)
	{
		ANKI_LOGE("SDL_GL_SetAttribute() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	if(init.m_debugContext)
	{
		if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG)
			!= 0)
		{
			ANKI_LOGE("SDL_GL_SetAttribute() failed");
			return ErrorCode::FUNCTION_FAILED;
		}
	}

	//
	// Create window
	//
	U32 flags = SDL_WINDOW_OPENGL;

	if(init.m_fullscreenDesktopRez)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	m_impl->m_window = SDL_CreateWindow(init.m_title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		init.m_width,
		init.m_height,
		flags);

	if(m_impl->m_window == nullptr)
	{
		ANKI_LOGE("SDL_CreateWindow() failed");
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

	//
	// Create context
	//
	m_impl->m_context = SDL_GL_CreateContext(m_impl->m_window);

	if(m_impl->m_context == nullptr)
	{
		ANKI_LOGE("SDL_GL_CreateContext() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	//
	// GLEW
	//
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		ANKI_LOGE("GLEW initialization failed");
		return ErrorCode::FUNCTION_FAILED;
	}
	glGetError();

	ANKI_LOGI("SDL window created");
	return ErrorCode::NONE;
}

//==============================================================================
void NativeWindow::destroy()
{
	if(m_impl != nullptr)
	{
		if(m_impl->m_context)
		{
			SDL_GL_DeleteContext(m_impl->m_context);
		}

		if(m_impl->m_window)
		{
			SDL_DestroyWindow(m_impl->m_window);
		}

		SDL_QuitSubSystem(INIT_SUBSYSTEMS);
		SDL_Quit();
	}

	m_alloc.deleteInstance(m_impl);
}

//==============================================================================
void NativeWindow::swapBuffers()
{
	ANKI_ASSERT(isCreated());
	SDL_GL_SwapWindow(m_impl->m_window);
}

//==============================================================================
Context NativeWindow::getCurrentContext()
{
	SDL_GLContext sdlCtx = SDL_GL_GetCurrentContext();
	return sdlCtx;
}

//==============================================================================
void NativeWindow::contextMakeCurrent(Context ctx)
{
	SDL_GLContext sdlCtx = ctx;
	SDL_GL_MakeCurrent(m_impl->m_window, sdlCtx);
}

} // end namespace anki

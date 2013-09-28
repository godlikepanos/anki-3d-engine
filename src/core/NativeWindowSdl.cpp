#include "anki/core/NativeWindowSdl.h"
#include "anki/core/Counters.h"
#include "anki/util/Exception.h"
#include <GL/glew.h>

namespace anki {

//==============================================================================
NativeWindow::~NativeWindow()
{}

//==============================================================================
void NativeWindow::create(NativeWindowInitializer& init)
{
	impl.reset(new NativeWindowImpl);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS 
		| SDL_INIT_GAMECONTROLLER) != 0)
	{
		throw ANKI_EXCEPTION("SDL_Init() failed");
	}
	
	//
	// Set GL attributes
	//
	if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, init.rgbaBits[0]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, init.rgbaBits[1]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, init.rgbaBits[2]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, init.rgbaBits[3]) != 0
		|| SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, init.depthBits) != 0
		|| SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, init.doubleBuffer) != 0
		|| SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MAJOR_VERSION, init.majorVersion) != 0
		|| SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_MINOR_VERSION, init.minorVersion) != 0
		|| SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0)
	{
		throw ANKI_EXCEPTION("SDL_GL_SetAttribute() failed");
	}

	if(init.debugContext)
	{
		if(SDL_GL_SetAttribute(
			SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG) != 0)
		{
			throw ANKI_EXCEPTION("SDL_GL_SetAttribute() failed");
		}
	}

	//
	// Create window
	//
	U32 flags = SDL_WINDOW_OPENGL;

	if(init.fullscreenDesktopRez)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	impl->window = SDL_CreateWindow(
    	init.title.c_str(), 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		init.width, init.height, flags);

	if(impl->window == nullptr)
	{
		throw ANKI_EXCEPTION("SDL_CreateWindow() failed");
	}

	// Set the size after loading a fullscreen window
	if(init.fullscreenDesktopRez)
	{
		int w, h;
		SDL_GetWindowSize(impl->window, &w, &h);

		width = w;
		height = h;
	}

	//
	// Create context
	//
	impl->context = SDL_GL_CreateContext(impl->window);

	if(impl->context == nullptr)
	{
		throw ANKI_EXCEPTION("SDL_GL_CreateContext() failed");
	}

	//
	// GLEW
	//
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		throw ANKI_EXCEPTION("GLEW initialization failed");
	}
	glGetError();
}

//==============================================================================
void NativeWindow::destroy()
{
	if(impl.get())
	{
		if(impl->context)
		{
			SDL_GL_DeleteContext(impl->context);
		}

		if(impl->window)
		{
			SDL_DestroyWindow(impl->window);
		}
	}

	impl.reset();
}

//==============================================================================
void NativeWindow::swapBuffers()
{
	ANKI_COUNTER_START_TIMER(C_SWAP_BUFFERS_TIME);
	ANKI_ASSERT(isCreated());
	SDL_GL_SwapWindow(impl->window);
	ANKI_COUNTER_STOP_TIMER_INC(C_SWAP_BUFFERS_TIME);
}

} // end namespace anki

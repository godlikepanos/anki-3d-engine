#include "anki/core/NativeWindowSdl.h"
#include "anki/core/Counters.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
NativeWindow::~NativeWindow()
{}

//==============================================================================
void NativeWindow::create(NativeWindowInitializer& initializer)
{
	impl.reset(new NativeWindowImpl);
	
	//
	// Set GL attributes
	//
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, initializer.rgbaBits[0]);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, initializer.rgbaBits[1]);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, initializer.rgbaBits[2]);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, initializer.rgbaBits[3]);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, initializer.depthBits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, initializer.doubleBuffer);

	if(initializer.debugContext)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, initializer.majorVersion);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, initializer.minorVersion);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 
		SDL_GL_CONTEXT_PROFILE_CORE);

	//
	// Create window
	//
	U32 flags = SDL_WINDOW_OPENGL;

	if(initializer.fullscreenDesktopRez)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	impl->window = SDL_CreateWindow(
    	initializer.title.c_str(), 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		initializer.width, initializer.height, flags);

	if(impl->window == nullptr)
	{
		throw ANKI_EXCEPTION("SDL_CreateWindow() failed");
	}

	// Set the size after loading a fullscreen window
	if(initializer.fullscreenDesktopRez)
	{
		int w, h;
		SDL_GetWindowSize(imp->window, &w, &h);

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
	glXSwapBuffers(impl->xDisplay, impl->xWindow);
	ANKI_COUNTER_STOP_TIMER_INC(C_SWAP_BUFFERS_TIME);
}

} // end namespace anki

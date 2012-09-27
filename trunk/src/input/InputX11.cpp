#include "anki/input/Input.h"
#include "anki/core/Logger.h"
#if ANKI_WINDOW_BACKEND_GLXX11
#	include "anki/core/NativeWindowGlxX11.h"
#elif ANKI_WINDOW_BACKEND_EGLX11
#	include "anki/core/NativeWindowEglX11.h"
#else
#	error "See file"
#endif

namespace anki {

//==============================================================================
static Bool eventsPending(Display* display)
{
	XFlush(display);
	if(XEventsQueued(display, QueuedAlready)) 
	{
		return true;
	}

	static struct timeval zero_time;
	int x11_fd;
	fd_set fdset;

	x11_fd = ConnectionNumber(display);
	FD_ZERO(&fdset);
	FD_SET(x11_fd, &fdset);
	if(select(x11_fd + 1, &fdset, NULL, NULL, &zero_time) == 1) 
	{
		return XPending(display);
	}

	return false;
}

//==============================================================================
void Input::init(NativeWindow* nativeWindow_)
{
	ANKI_ASSERT(nativeWindow == nullptr);
	nativeWindow = nativeWindow_;
	XSelectInput(nativeWindow->getNative().xDisplay, 
		nativeWindow->getNative().xWindow, 
		ExposureMask | ButtonPressMask | KeyPressMask);
	reset();
}

//==============================================================================
void Input::handleEvents()
{
	ANKI_ASSERT(nativeWindow != nullptr);

	// add the times a key is being pressed
	for(U32& k : keys)
	{
		if(k)
		{
			++k;
		}
	}
	for(U32& k : mouseBtns)
	{
		if(k)
		{
			++k;
		}
	}

	NativeWindowImpl& win = nativeWindow->getNative();
	Display* disp = win.xDisplay;
	while(eventsPending(disp))
	{
		XEvent event;
		XNextEvent(disp, &event);

		switch(event.type)
		{
		case KeyPress:
			ANKI_LOGI("Key pressed " << rand());
			break;
		default:
			ANKI_LOGW("Unknown X event");
			break;
		}
	}

#if 0

	SDL_Event event_;
	while(SDL_PollEvent(&event_))
	{
		switch(event_.type)
		{
		case SDL_KEYDOWN:
			keys[event_.key.keysym.scancode] = 1;
			break;

		case SDL_KEYUP:
			keys[event_.key.keysym.scancode] = 0;
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouseBtns[event_.button.button] = 1;
			break;

		case SDL_MOUSEBUTTONUP:
			mouseBtns[event_.button.button] = 0;
			break;

		case SDL_MOUSEMOTION:
		{
			Vec2 prevMousePosNdc(mousePosNdc);

			mousePos.x() = event_.button.x;
			mousePos.y() = event_.button.y;

			mousePosNdc.x() = (2.0 * mousePos.x()) /
				(F32)AppSingleton::get().getWindowWidth() - 1.0;
			mousePosNdc.y() = 1.0 - (2.0 * mousePos.y()) /
				(F32)AppSingleton::get().getWindowHeight();

			if(warpMouseFlag)
			{
				// the SDL_WarpMouse pushes an event in the event queue.
				// This check is so we wont process the event of the
				// SDL_WarpMouse function
				if(mousePosNdc == Vec2(0.0))
				{
					break;
				}

				uint w = AppSingleton::get().getWindowWidth();
				uint h = AppSingleton::get().getWindowHeight();
				SDL_WarpMouse(w / 2, h / 2);
			}

			mouseVelocity = mousePosNdc - prevMousePosNdc;
			break;
		}

		case SDL_QUIT:
			AppSingleton::get().quit(1);
			break;
		}
	}
#endif
}

} // end namespace anki

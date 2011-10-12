#include "anki/core/App.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"
#include "anki/input/Input.h"
#include <SDL/SDL.h>


namespace anki {


//==============================================================================
// init                                                                        =
//==============================================================================
void Input::init()
{
	ANKI_INFO("Initializing input...");
	warpMouseFlag = false;
	hideCursor = true;
	reset();
	ANKI_INFO("Input initialized");
}


//==============================================================================
// reset                                                                       =
//==============================================================================
void Input::reset(void)
{
	memset(&keys[0], 0, keys.size() * sizeof(short));
	memset(&mouseBtns[0], 0, mouseBtns.size() * sizeof(short));
	mousePosNdc = Vec2(0.0);
	mouseVelocity = Vec2(0.0);
}


//==============================================================================
// handleEvents                                                                =
//==============================================================================
void Input::handleEvents()
{
	if(hideCursor)
	{
		SDL_ShowCursor(SDL_DISABLE);
	}
	else
	{
		SDL_ShowCursor(SDL_ENABLE);
	}

	// add the times a key is bying pressed
	for(uint x=0; x<keys.size(); x++)
	{
		if(keys[x]) ++keys[x];
	}
	for(int x=0; x<8; x++)
	{
		if(mouseBtns[x]) ++mouseBtns[x];
	}


	mouseVelocity = Vec2(0.0);

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
					(float)AppSingleton::get().getWindowWidth() - 1.0;
				mousePosNdc.y() = 1.0 - (2.0 * mousePos.y()) /
					(float)AppSingleton::get().getWindowHeight();

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
}


} // end namespace

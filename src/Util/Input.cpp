#include "Input.h"
#include "Renderer.h"


namespace I {



/*
=======================================================================================================================================
data vars                                                                                                                             =
=======================================================================================================================================
*/
short keys [SDLK_LAST];  // shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continucely
short mouseBtns [8];    // mouse btns. Supporting 3 btns & wheel. Works the same as above
Vec2 mousePosNdc;    // the coords are in the ndc space
Vec2 mousePos;        // the coords are in the window space
Vec2 mouseVelocity;

bool warpMouse = false;
bool hideCursor = true;


/*
=======================================================================================================================================
reset                                                                                                                                 =
=======================================================================================================================================
*/
void Reset( void )
{
	memset( keys, 0, sizeof(keys) );
	memset(mouseBtns, 0, sizeof(mouseBtns) );
	mousePosNdc.setZero();
	mouseVelocity.setZero();
}


/*
=======================================================================================================================================
handleEvents                                                                                                                          =
=======================================================================================================================================
*/
void handleEvents()
{
	if( hideCursor ) SDL_ShowCursor( SDL_DISABLE );
	else              SDL_ShowCursor( SDL_ENABLE );

	// add the times a key is bying pressed
	for( int x=0; x<SDLK_LAST; x++ )
	{
		if( keys[x] ) ++keys[x];
	}
	for( int x=0; x<8; x++ )
	{
		if( mouseBtns[x] ) ++mouseBtns[x];
	}


	mouseVelocity.setZero();

	SDL_Event event_;
	while( SDL_PollEvent(&event_) )
	{
		switch( event_.type )
		{
			case SDL_KEYDOWN:
				keys[event_.key.keysym.sym] = 1;
				break;

			case SDL_KEYUP:
				keys[event_.key.keysym.sym] = 0;
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouseBtns[event_.button.button] = 1;
				break;

			case SDL_MOUSEBUTTONUP:
				mouseBtns[event_.button.button] = 0;
				break;

			case SDL_MOUSEMOTION:
			{
				Vec2 prevMousePosNdc( mousePosNdc );

				mousePos.x = event_.button.x;
				mousePos.y = event_.button.y;

				mousePosNdc.x = (2.0f * mousePos.x) / (float)App::windowW - 1.0f;
				mousePosNdc.y = 1.0f - (2.0f * mousePos.y) / (float)App::windowH;

				if( warpMouse )
				{
					// the SDL_WarpMouse pushes an event in the event queue. This check is so we wont process the event of the...
					// ...SDL_WarpMouse function
					if( mousePosNdc == Vec2::getZero() ) break;

					SDL_WarpMouse( App::windowW/2, App::windowH/2);
				}

				mouseVelocity = mousePosNdc - prevMousePosNdc;
				break;
			}

			case SDL_QUIT:
				App::quitApp(1);
				break;
		}
	}

	//cout << fixed << " velocity: " << mouseVelocity.x() << ' ' << mouseVelocity.y() << endl;
	//cout << fixed << mousePosNdc.x() << ' ' << mousePosNdc.y() << endl;
	//cout << crnt_keys[SDLK_m] << ' ' << prev_keys[SDLK_m] << "      " << keys[SDLK_m] << endl;
	//cout << mouseBtns[SDL_BUTTON_LEFT] << endl;

}



} // end namespace

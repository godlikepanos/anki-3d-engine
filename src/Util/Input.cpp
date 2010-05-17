#include <SDL.h>
#include "Input.h"
#include "Renderer.h"


namespace I {



//=====================================================================================================================================
// VARS                                                                                                                               =
//=====================================================================================================================================
Vec<short> keys( SDL_NUM_SCANCODES, 0 );
Vec<short> mouseBtns( 8, 0 );
Vec2 mousePosNdc;
Vec2 mousePos;
Vec2 mouseVelocity;

bool warpMouse = false;
bool hideCursor = true;


//=====================================================================================================================================
// reset                                                                                                                              =
//=====================================================================================================================================
void reset( void )
{
	DEBUG_ERR( keys.size() < 1 );
	DEBUG_ERR( mouseBtns.size() < 1 );
	memset( &keys[0], 0, keys.size()*sizeof(short) );
	memset( &mouseBtns[0], 0, mouseBtns.size()*sizeof(short) );
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
	else             SDL_ShowCursor( SDL_ENABLE );

	// add the times a key is bying pressed
	for( uint x=0; x<keys.size(); x++ )
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
				Vec2 prevMousePosNdc( mousePosNdc );

				mousePos.x = event_.button.x;
				mousePos.y = event_.button.y;

				mousePosNdc.x = (2.0 * mousePos.x) / (float)app->getWindowWidth() - 1.0;
				mousePosNdc.y = 1.0 - (2.0 * mousePos.y) / (float)app->getWindowHeight();

				if( warpMouse )
				{
					// the SDL_WarpMouse pushes an event in the event queue. This check is so we wont process the event of the...
					// ...SDL_WarpMouse function
					if( mousePosNdc == Vec2::getZero() ) break;

					SDL_WarpMouse( app->getWindowWidth()/2, app->getWindowHeight()/2);
				}

				mouseVelocity = mousePosNdc - prevMousePosNdc;
				break;
			}

			case SDL_QUIT:
				app->quitApp(1);
				break;
		}
	}

	//cout << fixed << " velocity: " << mouseVelocity.x() << ' ' << mouseVelocity.y() << endl;
	//cout << fixed << mousePosNdc.x() << ' ' << mousePosNdc.y() << endl;
	//cout << crnt_keys[SDLK_m] << ' ' << prev_keys[SDLK_m] << "      " << keys[SDLK_m] << endl;
	//cout << mouseBtns[SDL_BUTTON_LEFT] << endl;

}



} // end namespace

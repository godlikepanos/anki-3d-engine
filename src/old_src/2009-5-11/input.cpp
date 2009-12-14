#include "input.h"


namespace i {



/*
=================================================================================================================================================================
data vars                                                                                                                                                       =
=================================================================================================================================================================
*/
short keys [SDLK_LAST];  // shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continucely
short mouse_btns [8];    // mouse btns. Supporting 3 btns & wheel. Works the same as above
vec2_t mouse_pos_ndc;    // the coords are in the ndc space
vec2_t mouse_pos;        // the coords are in the window space
vec2_t mouse_velocity;


/*
=================================================================================================================================================================
Reset                                                                                                                                                           =
=================================================================================================================================================================
*/
void Reset( void )
{
	memset( keys, 0, sizeof(keys) );
	memset(mouse_btns, 0, sizeof(mouse_btns) );
	mouse_pos_ndc.SetZero();
	mouse_velocity.SetZero();
}


/*
=================================================================================================================================================================
HandleEvents                                                                                                                                                    =
=================================================================================================================================================================
*/
void HandleEvents()
{
	SDL_Event event_;

	// add the times a key is bying pressed
	for( int x=0; x<SDLK_LAST; x++ )
	{
		if( keys[x] ) ++keys[x];
	}
	for( int x=0; x<8; x++ )
	{
		if( mouse_btns[x] ) ++mouse_btns[x];
	}

	mouse_velocity.SetZero();
	vec2_t prev_mouse_pos_ndc( mouse_pos_ndc );

	while (SDL_PollEvent(&event_))
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
				mouse_btns[event_.button.button] = 1;
				break;

			case SDL_MOUSEBUTTONUP:
				mouse_btns[event_.button.button] = 0;
				break;

			case SDL_MOUSEMOTION:
				mouse_pos.x = event_.button.x;
				mouse_pos.y = event_.button.y;

				mouse_pos_ndc.x = (2.0f * mouse_pos.x) / (float)r::w - 1.0f;
				mouse_pos_ndc.y = 1.0f - (2.0f * mouse_pos.y) / (float)r::h;

				mouse_velocity = mouse_pos_ndc - prev_mouse_pos_ndc;

				break;

			case SDL_QUIT:
				hndl::QuitApp(1);
				break;
		}
	}

	//SDL_WarpMouse( renderer.w/2, renderer.h/2);

	//cout << fixed << " velocity: " << mouse_velocity.x() << ' ' << mouse_velocity.y() << endl;
	//cout << fixed << mouse_pos_ndc.x() << ' ' << mouse_pos_ndc.y() << endl;
	//cout << crnt_keys[SDLK_m] << ' ' << prev_keys[SDLK_m] << "      " << keys[SDLK_m] << endl;
	//cout << mouse_btns[SDL_BUTTON_LEFT] << endl;

}



} // end namespace

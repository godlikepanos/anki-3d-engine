#include "app.h"

namespace app { // begin of namespace


static SDL_Surface* main_surf;
static SDL_Surface* icon_image;

uint timer_tick = 1000/40; // in ms. 1000/Hz
static uint time = 0;

uint desktop_w;
uint desktop_h;


/*
=======================================================================================================================================
InitWindow                                                                                                                            =
=======================================================================================================================================
*/
void InitWindow( int w, int h, const char* window_caption )
{
	PRINT( "SDL window initializing..." );
	SDL_Init( SDL_INIT_VIDEO );

	// get desctop size
	const SDL_VideoInfo* info = SDL_GetVideoInfo();
	desktop_w = info->current_w;
	desktop_h = info->current_h;

	// the icon
	icon_image = SDL_LoadBMP("gfx/icon.bmp");
	if( icon_image == NULL )
	{
		ERROR( "Cannot load window icon" );
	}
	else
	{
		Uint32 colorkey = SDL_MapRGB( icon_image->format, 255, 0, 255 );
		SDL_SetColorKey( icon_image, SDL_SRCCOLORKEY, colorkey );
		SDL_WM_SetIcon( icon_image, NULL );
	}

	// set GL attribs
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 8 ); // WARNING: Set this only in deffered shading
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );

	// set the surface
	main_surf = SDL_SetVideoMode( w, h, 24, SDL_HWSURFACE | SDL_OPENGL );

	// move the window
#ifdef WIN32
	SDL_SysWMinfo i;
	SDL_VERSION( &i.version );
	if( SDL_GetWMInfo(&i) )
	{
		HWND hwnd = i.window;
		SetWindowPos( hwnd, HWND_TOP, 10, 25, w, h, NULL );
	}
#endif

	SDL_WM_SetCaption( window_caption, NULL );

	PRINT( "SDL window initialization ends" );
}


/*
=======================================================================================================================================
TogleFullScreen                                                                                                                       =
=======================================================================================================================================
*/
void TogleFullScreen()
{
	SDL_WM_ToggleFullScreen( main_surf );
}


/*
=======================================================================================================================================
QuitApp                                                                                                                               =
=======================================================================================================================================
*/
void QuitApp( int code )
{
	SDL_FreeSurface( main_surf );
	SDL_Quit();
	exit(code);
}


/*
=======================================================================================================================================
WaitForNextFrame                                                                                                                      =
=======================================================================================================================================
*/
void WaitForNextFrame()
{
	uint now = SDL_GetTicks();

	if( now - time < timer_tick )
	{
		// the new time after the SDL_Delay will be...
		time += timer_tick;
		// sleep a little
		SDL_Delay( time - now);
	}
	else
		time = now;

}


} // end of namespace

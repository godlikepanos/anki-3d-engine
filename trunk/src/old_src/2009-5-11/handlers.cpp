#include "handlers.h"

namespace hndl { // begin of namespace


SDL_Surface* main_surf;
SDL_Surface* icon_image;

uint timer_tick = 25; // in ms


// InitWindow
void InitWindow( int w, int h, const char* window_caption )
{
	SDL_Init( SDL_INIT_VIDEO );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// the icon
	icon_image = SDL_LoadBMP("gfx/icon.bmp");
	Uint32 colorkey = SDL_MapRGB( icon_image->format, 255, 0, 255 );
	SDL_SetColorKey( icon_image, SDL_SRCCOLORKEY, colorkey );
	SDL_WM_SetIcon( icon_image, NULL );

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
}


// TogleFullScreen
void TogleFullScreen()
{
	SDL_WM_ToggleFullScreen( main_surf );
}


// QuitApp
void QuitApp( int code )
{
	SDL_FreeSurface( main_surf );
	SDL_Quit();
	//mem_PrintInfo( MEM_PRINT_ALL );
	exit(code);
}


// WaitForNextTick
void WaitForNextFrame()
{
	static uint nextTime = SDL_GetTicks();

	uint now = SDL_GetTicks();
	if( nextTime > now )
		SDL_Delay(nextTime - now);

	nextTime += timer_tick;
}


} // end of namespace

#include <GL/glew.h>
#include "app.h"

namespace app { // begin of namespace


static SDL_Surface* main_surf;
static SDL_Surface* icon_image;

uint windowW = 1280;
uint windowH = 800;

uint timer_tick = 1000/40; // in ms. 1000/Hz
static uint time = 0;

uint desktopW;
uint desktopH;


/*
=======================================================================================================================================
initWindow                                                                                                                            =
=======================================================================================================================================
*/
void initWindow()
{
	PRINT( "SDL window initializing..." );
	SDL_Init( SDL_INIT_VIDEO );

	// print driver name
	char charBuff [256];
	if( SDL_VideoDriverName(charBuff, sizeof(charBuff)) != NULL )
	{
		PRINT( "The video driver name is " << charBuff );
	}
	else
	{
		ERROR( "Failed to obtain the video driver name" );
	}


	// get desktop size
	const SDL_VideoInfo* info = SDL_GetVideoInfo();
	desktopW = info->current_w;
	desktopH = info->current_h;

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
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 8 ); // WARNING: Set this only in deferred shading
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );

	// set the surface
	main_surf = SDL_SetVideoMode( windowW, windowH, 24, SDL_HWSURFACE | SDL_OPENGL );

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

	SDL_WM_SetCaption( "AnKi Engine", NULL );


	PRINT( "SDL window initialization ends" );
}


/*
=======================================================================================================================================
togleFullScreen                                                                                                                       =
=======================================================================================================================================
*/
void togleFullScreen()
{
	SDL_WM_ToggleFullScreen( main_surf );
}


/*
=======================================================================================================================================
quitApp                                                                                                                               =
=======================================================================================================================================
*/
void quitApp( int code )
{
	SDL_FreeSurface( main_surf );
	SDL_Quit();
	exit(code);
}


/*
=======================================================================================================================================
waitForNextFrame                                                                                                                      =
=======================================================================================================================================
*/
void waitForNextFrame()
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


//=====================================================================================================================================
// printAppInfo                                                                                                                       =
//=====================================================================================================================================
void printAppInfo()
{
	cout << "App info: ";
	#if defined( _DEBUG_ )
		cout << "Debug ";
	#else
		cout << "Release ";
	#endif
	cout << "GLEW_" << glewGetString(GLEW_VERSION) << " ";
	const SDL_version* v = SDL_Linked_Version();
	cout << "SDL_" << int(v->major) << '.' << int(v->minor) << '.' <<  int(v->patch) << endl;
}


} // end of namespace

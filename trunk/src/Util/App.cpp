#include <GL/glew.h>
#include <sstream>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "App.h"
#include "Scene.h"
#include "MainRenderer.h"
#include <boost/filesystem.hpp>

bool App::isCreated = false;


//======================================================================================================================
// parseCommandLineArgs                                                                                                =
//======================================================================================================================
void App::parseCommandLineArgs(int argc, char* argv[])
{
	for(int i=1; i<argc; i++)
	{
		char* arg = argv[i];
		if(strcmp(arg, "--terminal-coloring") == 0)
		{
			terminalColoringEnabled = true;
		}
		else if(strcmp(arg, "--no-terminal-coloring") == 0)
		{
			terminalColoringEnabled = false;
		}
		else
		{
			FATAL("Incorrect command line argument \"" << arg << "\"");
		}
	}

}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
App::App(int argc, char* argv[]):
	windowW(1280),
	windowH(720),
	terminalColoringEnabled(true),
	fullScreenFlag(false)
{
	app = this;

	parseCommandLineArgs(argc, argv);

	printAppInfo();

	if(isCreated)
		FATAL("You cannot init a second App instance");

	isCreated = true;

	// dirs
	settingsPath = filesystem::path(getenv("HOME")) / ".anki";
	if(!filesystem::exists(settingsPath))
	{
		INFO("Creating settings dir \"" << settingsPath << "\"");
		filesystem::create_directory(settingsPath);
	}

	cachePath = settingsPath / "cache";
	if(filesystem::exists(cachePath))
	{
		filesystem::remove_all(cachePath);
	}

	INFO("Creating cache dir \"" << cachePath << "\"");
	filesystem::create_directory(cachePath);


	scene = new Scene;
	mainRenderer = new MainRenderer;
	activeCam = NULL;

	timerTick = 1000/40; // in ms. 1000/Hz
	time = 0;
}


//======================================================================================================================
// initWindow                                                                                                          =
//======================================================================================================================
void App::initWindow()
{
	INFO("SDL window initializing...");

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
		FATAL("Failed to init SDL_VIDEO");

	// print driver name
	const char* driverName = SDL_GetCurrentVideoDriver();
	if(driverName != NULL)
	{
		INFO("Video driver name: " << driverName);
	}
	else
	{
		ERROR("Failed to obtain the video driver name");
	}

	// set GL attribs
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8); // WARNING: Set this low only in deferred shading
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// OpenWindow
	windowId = SDL_CreateWindow("AnKi Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH,
	                             SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if(!windowId)
		FATAL("Cannot create main window");

	glContext = SDL_GL_CreateContext(windowId);


	// the icon
	iconImage = SDL_LoadBMP("gfx/icon.bmp");
	if(iconImage == NULL)
	{
		ERROR("Cannot load window icon");
	}
	else
	{
		Uint32 colorkey = SDL_MapRGB(iconImage->format, 255, 0, 255);
		SDL_SetColorKey(iconImage, SDL_SRCCOLORKEY, colorkey);
		//SDL_WM_SetIcon(iconImage, NULL);
		SDL_SetWindowIcon(windowId, iconImage);
	}

	INFO("SDL window initialization ends");
}


//======================================================================================================================
// togleFullScreen                                                                                                     =
//======================================================================================================================
void App::togleFullScreen()
{
	//SDL_WM_ToggleFullScreen(mainSurf);
	SDL_SetWindowFullscreen(windowId, fullScreenFlag);
	fullScreenFlag = !fullScreenFlag;
}


//======================================================================================================================
// swapBuffers                                                                                                         =
//======================================================================================================================
void App::swapBuffers()
{
	//SDL_GL_SwapBuffers();
	SDL_GL_SwapWindow(windowId);
}


//======================================================================================================================
// quit                                                                                                             =
//======================================================================================================================
void App::quit(int code)
{
	SDL_FreeSurface(iconImage);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(windowId);
	SDL_Quit();
	exit(code);
}


//======================================================================================================================
// waitForNextFrame                                                                                                    =
//======================================================================================================================
void App::waitForNextFrame()
{
	uint now = SDL_GetTicks();

	if(now - time < timerTick)
	{
		// the new time after the SDL_Delay will be...
		time += timerTick;
		// sleep a little
		SDL_Delay(time - now);
	}
	else
		time = now;

}


//======================================================================================================================
// printAppInfo                                                                                                        =
//======================================================================================================================
#if !defined(REVISION)
	#define REVISION "unknown"
#endif

void App::printAppInfo()
{
	stringstream msg;
	msg << "App info: debugging ";
	#if defined(DEBUG_ENABLED)
		msg << "on, ";
	#else
		msg << "off, ";
	#endif
	msg << "platform ";
	#if defined(PLATFORM_LINUX)
		msg << "Linux, ";
	#elif defined(PLATFORM_WIN)
		msg << "Windows, ";
	#else
		#error "See file"
	#endif
	msg << "GLEW " << glewGetString(GLEW_VERSION) << ", ";
	const SDL_version* v = SDL_Linked_Version();
	msg << "SDL " << int(v->major) << '.' << int(v->minor) << '.' << int(v->patch) << ", ";
	v = IMG_Linked_Version();
	msg << "SDL_image " << int(v->major) << '.' << int(v->minor) << '.' << int(v->patch) << ", ";
	msg << "build date " __DATE__ << ", ";
	msg << "rev " << REVISION;

	INFO(msg.str());
}


//======================================================================================================================
// getDesktopWidth                                                                                                     =
//======================================================================================================================
uint App::getDesktopWidth() const
{
	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(&mode);
	return mode.w;
}


//======================================================================================================================
// getDesktopHeight                                                                                                    =
//======================================================================================================================
uint App::getDesktopHeight() const
{
	SDL_DisplayMode mode;
	SDL_GetDesktopDisplayMode(&mode);
	return mode.h;
}


//======================================================================================================================
// getTicks                                                                                                            =
//======================================================================================================================
uint App::getTicks()
{
	return SDL_GetTicks();
}


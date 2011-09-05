#include "App.h"
#include "Logger.h"
#include "core/Globals.h"
#include "util/Exception.h"
#include <GL/glew.h>
#include <sstream>
#include <SDL/SDL.h>
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>


//==============================================================================
// handleMessageHanlderMsgs                                                    =
//==============================================================================
void App::handleMessageHanlderMsgs(const char* file, int line,
	const char* func, Logger::MessageType type, const char* msg)
{
	std::ostream* out = NULL;
	const char* x = NULL;

	switch(type)
	{
		case Logger::MT_NORMAL:
			out = &std::cout;
			x = "Info";
			break;

		case Logger::MT_ERROR:
			out = &std::cerr;
			x = "Error";
			break;

		case Logger::MT_WARNING:
			out = &std::cerr;
			x = "Warn";
			break;
	}

	(*out) << "(" << file << ":" << line << " "<< func <<
		") " << x << ": " << msg << std::flush;
}


//==============================================================================
// parseCommandLineArgs                                                        =
//==============================================================================
void App::parseCommandLineArgs(int argc, char* argv[])
{
	for(int i = 1; i < argc; i++)
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
			std::cerr << "Incorrect command line argument \"" << arg << "\"" <<
				std::endl;
			abort();
		}
	}
}


//==============================================================================
// init                                                                        =
//==============================================================================
void App::init(int argc, char* argv[])
{
	windowW = 1280;
	windowH = 720;
	terminalColoringEnabled = true,
	fullScreenFlag = false;

	// send output to handleMessageHanlderMsgs
	LoggerSingleton::get().connect(&App::handleMessageHanlderMsgs, this);

	parseCommandLineArgs(argc, argv);
	printAppInfo();
	initDirs();
	initWindow();

	// other
	activeCam = NULL;
	timerTick = 1.0 / 60.0; // in sec. 1.0 / period
}


//==============================================================================
// initWindow                                                                  =
//==============================================================================
void App::initWindow()
{
	INFO("SDL window initializing...");

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw EXCEPTION("Failed to init SDL_VIDEO");
	}

	// print driver name
	const char* driverName = SDL_GetCurrentVideoDriver();
	if(driverName != NULL)
	{
		INFO("Video driver name: " << driverName);
	}

	// set GL attribs
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	// WARNING: Set this low only in deferred shading
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// OpenWindow
	windowId = SDL_CreateWindow("AnKi 3D Engine", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, windowW, windowH,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if(!windowId)
	{
		throw EXCEPTION("Cannot create main window");
	}

	glContext = SDL_GL_CreateContext(windowId);

	// the icon
	iconImage = SDL_LoadBMP("gfx/icon.bmp");
	if(iconImage == NULL)
	{
		WARNING("Cannot load window icon");
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


//==============================================================================
// initDirs                                                                    =
//==============================================================================
void App::initDirs()
{
	settingsPath = boost::filesystem::path(getenv("HOME")) / ".anki";
	if(!boost::filesystem::exists(settingsPath))
	{
		INFO("Creating settings dir \"" << settingsPath.string() << "\"");
		boost::filesystem::create_directory(settingsPath);
	}

	cachePath = settingsPath / "cache";
	if(boost::filesystem::exists(cachePath))
	{
		INFO("Deleting dir \"" << cachePath.string() << "\"");
		boost::filesystem::remove_all(cachePath);
	}

	INFO("Creating cache dir \"" << cachePath.string() << "\"");
	boost::filesystem::create_directory(cachePath);
}


//==============================================================================
// togleFullScreen                                                             =
//==============================================================================
void App::togleFullScreen()
{
	//SDL_WM_ToggleFullScreen(mainSurf);
	SDL_SetWindowFullscreen(windowId, fullScreenFlag ? SDL_TRUE : SDL_FALSE);
	fullScreenFlag = !fullScreenFlag;
}


//==============================================================================
// swapBuffers                                                                 =
//==============================================================================
void App::swapBuffers()
{
	//SDL_GL_SwapBuffers();
	SDL_GL_SwapWindow(windowId);
}


//==============================================================================
// quit                                                                        =
//==============================================================================
void App::quit(int code)
{
	SDL_FreeSurface(iconImage);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(windowId);
	SDL_Quit();
	exit(code);
}


//==============================================================================
// printAppInfo                                                                =
//==============================================================================
#if !defined(REVISION)
#	define REVISION "unknown"
#endif

void App::printAppInfo()
{
	std::stringstream msg;
	msg << "App info: ";
#if defined(NDEBUG)
	msg << "Release";
#else
	msg << "Debug";
#endif
	msg << " build, ";
	msg << "platform ";
#if defined(PLATFORM_LINUX)
	msg << "Linux, ";
#elif defined(PLATFORM_WIN)
	msg << "Windows, ";
#else
#	error "See file"
#endif
	msg << "GLEW " << glewGetString(GLEW_VERSION) << ", ";
	const SDL_version* v = SDL_Linked_Version();
	msg << "SDL " << int(v->major) << '.' << int(v->minor) << '.' <<
		int(v->patch) << ", " << "build date " __DATE__ << ", " <<
		"rev " << REVISION;

	INFO(msg.str());
}


//==============================================================================
// getDesktopWidth                                                             =
//==============================================================================
uint App::getDesktopWidth() const
{
	SDL_DisplayMode mode;
	/// @todo re-enable it
	//SDL_GetDesktopDisplayMode(&mode);
	return mode.w;
}


//==============================================================================
// getDesktopHeight                                                            =
//==============================================================================
uint App::getDesktopHeight() const
{
	SDL_DisplayMode mode;
	/// @todo re-enable it
	//SDL_GetDesktopDisplayMode(&mode);
	return mode.h;
}

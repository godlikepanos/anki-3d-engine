#include "anki/core/App.h"
#include "anki/core/Logger.h"
#include "anki/util/Exception.h"
#include "anki/util/Platform.h"
#include "anki/util/Filesystem.h"
#include <GL/glew.h>
#include <sstream>
#include <SDL/SDL.h>
#include <iostream>
#include <iomanip>
#include <boost/algorithm/string.hpp>

namespace anki {

//==============================================================================
void App::handleLoggerMessages(const Logger::Info& info)
{
	std::ostream* out = NULL;
	const char* x = NULL;

	switch(info.type)
	{
	case Logger::LMT_NORMAL:
		out = &std::cout;
		x = "Info";
		break;

	case Logger::LMT_ERROR:
		out = &std::cerr;
		x = "Error";
		break;

	case Logger::LMT_WARNING:
		out = &std::cerr;
		x = "Warn";
		break;
	}

	(*out) << "(" << info.file << ":" << info.line << " "<< info.func 
		<< ") " << x << ": " << info.msg << std::endl;
}

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
			std::cerr << "Incorrect command line argument: " << arg
				<< std::endl;
			abort();
		}
	}
}

//==============================================================================
void App::init(int argc, char* argv[])
{
	windowW = 1280;
	windowH = 720;
	terminalColoringEnabled = true,
	fullScreenFlag = false;

	// send output to handleMessageHanlderMsgs
	ANKI_CONNECT(&LoggerSingleton::get(), messageRecieved, 
		this, handleLoggerMessages);

	parseCommandLineArgs(argc, argv);
	printAppInfo();
	initDirs();
	//initWindow();
	//SDL_Init(SDL_INIT_INPUT)

	// other
	activeCam = NULL;
	timerTick = 1.0 / 60.0; // in sec. 1.0 / period
}

//==============================================================================
void App::initWindow()
{
	ANKI_LOGI("SDL window initializing...");

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw ANKI_EXCEPTION("Failed to init SDL_VIDEO");
	}

	// print driver name
	const char* driverName = SDL_GetCurrentVideoDriver();
	if(driverName != NULL)
	{
		ANKI_LOGI("Video driver name: " << driverName);
	}

	// set GL attribs
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
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
		throw ANKI_EXCEPTION("Cannot create main window");
	}

	glContext = SDL_GL_CreateContext(windowId);

	// the icon
	iconImage = SDL_LoadBMP("gfx/icon.bmp");
	if(iconImage == NULL)
	{
		ANKI_LOGW("Cannot load window icon");
	}
	else
	{
		Uint32 colorkey = SDL_MapRGB(iconImage->format, 255, 0, 255);
		SDL_SetColorKey(iconImage, SDL_SRCCOLORKEY, colorkey);
		//SDL_WM_SetIcon(iconImage, NULL);
		SDL_SetWindowIcon(windowId, iconImage);
	}

	ANKI_LOGI("SDL window initialization ends");
}

//==============================================================================
void App::initDirs()
{
	settingsPath = std::string(getenv("HOME")) + "/.anki";
	if(!directoryExists(settingsPath.c_str()))
	{
		ANKI_LOGI("Creating settings dir: " << settingsPath);
		createDirectory(settingsPath.c_str());
	}

	cachePath = settingsPath + "/cache";
	if(directoryExists(cachePath.c_str()))
	{
		ANKI_LOGI("Deleting dir: " << cachePath);
		removeDirectory(cachePath.c_str());
	}

	ANKI_LOGI("Creating cache dir: " << cachePath);
	createDirectory(cachePath.c_str());
}

//==============================================================================
void App::togleFullScreen()
{
	//SDL_WM_ToggleFullScreen(mainSurf);
	SDL_SetWindowFullscreen(windowId, fullScreenFlag ? SDL_TRUE : SDL_FALSE);
	fullScreenFlag = !fullScreenFlag;
}

//==============================================================================
void App::swapBuffers()
{
	//SDL_GL_SwapBuffers();
	SDL_GL_SwapWindow(windowId);
}

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
#if !defined(ANKI_REVISION)
#	define ANKI_REVISION "unknown"
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
	msg << "platform ID " << ANKI_PLATFORM << ", ";
	msg << "compiler ID " << ANKI_COMPILER << ", ";
	msg << "GLEW " << glewGetString(GLEW_VERSION) << ", ";
	const SDL_version* v = SDL_Linked_Version();
	msg << "SDL " << int(v->major) << '.' << int(v->minor) << '.' 
		<< int(v->patch) << ", " << "build date " __DATE__ << ", " 
		<< "rev " << ANKI_REVISION;

	ANKI_LOGI(msg.str());
}

//==============================================================================
uint App::getDesktopWidth() const
{
	/*SDL_DisplayMode mode;
	/// @todo re-enable it
	//SDL_GetDesktopDisplayMode(&mode);
	return mode.w;*/
	return 0;
}

//==============================================================================
uint App::getDesktopHeight() const
{
	/*SDL_DisplayMode mode;
	/// @todo re-enable it
	//SDL_GetDesktopDisplayMode(&mode);
	return mode.h;*/
	return 0;
}

} // end namespace

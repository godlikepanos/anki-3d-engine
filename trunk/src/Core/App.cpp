#include <GL/glew.h>
#include <sstream>
#include <SDL/SDL.h>
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "App.h"
#include "RendererInitializer.h"
#include "MainRenderer.h"
#include "ScriptingEngine.h"
#include "StdinListener.h"
#include "Input.h"
#include "Logger.h"


//======================================================================================================================
// handleMessageHanlderMsgs                                                                                            =
//======================================================================================================================
void App::handleMessageHanlderMsgs(const char* file, int line, const char* func, const char* msg)
{
	if(boost::find_first(msg, "Warning") || boost::find_first(msg, "Error"))
	{
		std::cerr << "(" << file << ":" << line << " "<< func << ") " << msg << std::flush;
	}
	else
	{
		std::cout << "(" << file << ":" << line << " "<< func << ") " << msg << std::flush;
	}
}


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
			std::cerr << "Incorrect command line argument \"" << arg << "\"" << std::endl;
			abort();
		}
	}
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void App::init(int argc, char* argv[])
{
	windowW = 1280;
	windowH = 720;
	terminalColoringEnabled = true,
	fullScreenFlag = false;

	// send output to handleMessageHanlderMsgs
	LoggerSingleton::getInstance().getSignal().connect(boost::bind(&App::handleMessageHanlderMsgs,
	                                                               this, _1, _2, _3, _4));

	INFO("Initializing the engine...");

	parseCommandLineArgs(argc, argv);

	printAppInfo();

	// dirs
	initDirs();

	// create the subsystems. WATCH THE ORDER
	const char* commonPythonCode =
	"import sys\n"
	"from Anki import *\n"
	"\n"
	"class StdoutCatcher:\n"
	"\tdef write(self, str_):\n"
	"\t\tif str_ == \"\\n\": return\n"
	"\t\tline = sys._getframe(1).f_lineno\n"
	"\t\tfile = sys._getframe(1).f_code.co_filename\n"
	"\t\tfunc = sys._getframe(1).f_code.co_name\n"
	"\t\tLoggerSingleton.getInstance().write(file, line, func, str_ + \"\\n\")\n"
	"\n"
	"class StderrCatcher:\n"
	"\tdef write(self, str_):\n"
	"\t\tline = sys._getframe(1).f_lineno\n"
	"\t\tfile = sys._getframe(1).f_code.co_filename\n"
	"\t\tfunc = sys._getframe(1).f_code.co_name\n"
	"\t\tLoggerSingleton.getInstance().write(file, line, func, str_)\n"
	"\n"
	"sys.stdout = StdoutCatcher()\n"
	"sys.stderr = StderrCatcher()\n";
	ScriptingEngineSingleton::getInstance().execScript(commonPythonCode);

	StdinListenerSingleton::getInstance().start();

	initWindow();
	initRenderer();

	// other
	activeCam = NULL;
	timerTick = 1000 / 40; // in ms. 1000/Hz
	time = 0;

	INFO("Engine initialization ends");
}


//======================================================================================================================
// initWindow                                                                                                          =
//======================================================================================================================
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
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8); // WARNING: Set this low only in deferred shading
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// OpenWindow
	windowId = SDL_CreateWindow("AnKi 3D Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowW, windowH,
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


//======================================================================================================================
// initDirs                                                                                                            =
//======================================================================================================================
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


//======================================================================================================================
// initRenderer                                                                                                        =
//======================================================================================================================
void App::initRenderer()
{
	RendererInitializer initializer;
	initializer.ms.ez.enabled = false;
	initializer.dbg.enabled = true;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = true;
	initializer.is.sm.resolution = 1024;
	initializer.is.sm.level0Distance = 5.0;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.25;
	initializer.pps.hdr.blurringDist = 1.0;
	initializer.pps.hdr.blurringIterationsNum = 2;
	initializer.pps.hdr.exposure = 4.0;
	initializer.pps.ssao.blurringIterationsNum = 2;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.renderingQuality = 0.4;
	initializer.mainRendererQuality = 1.0;
	MainRendererSingleton::getInstance().init(initializer);
}


//======================================================================================================================
// togleFullScreen                                                                                                     =
//======================================================================================================================
void App::togleFullScreen()
{
	//SDL_WM_ToggleFullScreen(mainSurf);
	SDL_SetWindowFullscreen(windowId, fullScreenFlag ? SDL_TRUE : SDL_FALSE);
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
// printAppInfo                                                                                                        =
//======================================================================================================================
#if !defined(REVISION)
	#define REVISION "unknown"
#endif

void App::printAppInfo()
{
	std::stringstream msg;
	msg << "App info: Build: ";
	#if defined(NDEBUG)
		msg << "release, ";
	#else
		msg << "debug, ";
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
	/// @todo re-enable it
	//SDL_GetDesktopDisplayMode(&mode);
	return mode.w;
}


//======================================================================================================================
// getDesktopHeight                                                                                                    =
//======================================================================================================================
uint App::getDesktopHeight() const
{
	SDL_DisplayMode mode;
	/// @todo re-enable it
	//SDL_GetDesktopDisplayMode(&mode);
	return mode.h;
}


//======================================================================================================================
// execStdinScpripts                                                                                                   =
//======================================================================================================================
void App::execStdinScpripts()
{
	while(1)
	{
		std::string cmd = StdinListenerSingleton::getInstance().getLine();

		if(cmd.length() < 1)
		{
			break;
		}

		try
		{
			ScriptingEngineSingleton::getInstance().execScript(cmd.c_str(), "command line input");
		}
		catch(Exception& e)
		{
			ERROR(e.what());
		}
	}
}

#include <GL/glew.h>
#include <sstream>
#include <SDL/SDL.h>
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include "App.h"
#include "Scene.h"
#include "MainRenderer.h"
#include "ScriptingEngine.h"
#include "StdinListener.h"
#include "MessageHandler.h"
#include "Messaging.h"
#include "Input.h"


App* app = NULL; ///< The only global var. App constructor sets it


bool App::isCreated = false;


//======================================================================================================================
// handleMessageHanlderMsgs                                                                                            =
//======================================================================================================================
void App::handleMessageHanlderMsgs(const char* file, int line, const char* func, const std::string& msg)
{
	std::cout << "(" << file << ":" << line << " "<< func << ") " << msg << std::endl;
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
// Constructor                                                                                                         =
//======================================================================================================================
App::App(int argc, char* argv[], Object* parent):
	Object(parent),
	windowW(1280),
	windowH(720),
	terminalColoringEnabled(true),
	fullScreenFlag(false)
{
	app = this;

	parseCommandLineArgs(argc, argv);

	messageHandler = new MessageHandler(this);
	// send output to handleMessageHanlderMsgs
	messageHandler->getSignal().connect(boost::bind(&App::handleMessageHanlderMsgs, this, _1, _2, _3, _4));

	printAppInfo();

	if(isCreated)
	{
		throw EXCEPTION("You cannot init a second App instance");
	}

	isCreated = true;

	// dirs
	settingsPath = boost::filesystem::path(getenv("HOME")) / ".anki";
	if(!boost::filesystem::exists(settingsPath))
	{
		INFO("Creating settings dir \"" + settingsPath.string() + "\"");
		boost::filesystem::create_directory(settingsPath);
	}

	cachePath = settingsPath / "cache";
	if(boost::filesystem::exists(cachePath))
	{
		INFO("Deleting dir \"" + cachePath.string() + "\"");
		boost::filesystem::remove_all(cachePath);
	}

	INFO("Creating cache dir \"" + cachePath.string() + "\"");
	boost::filesystem::create_directory(cachePath);

	// create the subsystems. WATCH THE ORDER
	scriptingEngine = new ScriptingEngine(this);
	scriptingEngine->exposeVar("app", this);

	const char* commonPythonCode =
	"import sys\n"
	"from Anki import *\n"
	"\n"
	"class StdoutCatcher:\n"
	"\tdef write(self, str):\n"
	"\t\tline = sys._getframe(2).f_lineno\n"
	"\t\tfile = sys._getframe(2).f_code.co_filename\n"
	"\t\tfunc = sys._getframe(2).f_code.co_name\n"
	"\t\tapp.getMessageHandler().write(fname, line, func, str)\n"
	"\n"
	"class StderrCatcher:\n"
	"\tdef write(self, str):\n"
	"\t\tline = sys._getframe(2).f_lineno\n"
	"\t\tfile = sys._getframe(2).f_code.co_filename\n"
	"\t\tfunc = sys._getframe(2).f_code.co_name\n"
	"\t\tapp.getMessageHandler().write(fname, line, func, str)\n"
	"\n"
	"sys.stdout = StdoutCatcher\n"
	"sys.stderr = StderrCatcher\n";

	scriptingEngine->execScript(commonPythonCode);
	mainRenderer = new MainRenderer(this);
	scene = new Scene(this);
	stdinListener = new StdinListener(this);
	stdinListener->start();
	input = new Input(this);

	// other
	activeCam = NULL;
	timerTick = 1000 / 40; // in ms. 1000/Hz
	time = 0;
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
		INFO("Video driver name: " + driverName);
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
	{
		time = now;
	}
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
	msg << "App info: debugging ";
	#if DEBUG_ENABLED == 1
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


//======================================================================================================================
// execStdinScpripts                                                                                                   =
//======================================================================================================================
void App::execStdinScpripts()
{
	while(1)
	{
		std::string cmd = app->getStdinLintener().getLine();

		if(cmd.length() < 1)
		{
			break;
		}

		app->getScriptingEngine().execScript(cmd.c_str(), "command line input");
	}
}

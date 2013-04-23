#include "anki/core/App.h"
#include "anki/core/Logger.h"
#include "anki/util/Exception.h"
#include "anki/util/Filesystem.h"
#include "anki/util/System.h"
#include "anki/Config.h"
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <execinfo.h>
#include <signal.h>

namespace anki {

//==============================================================================
/// Segfault signal handler
static void handler(int sig)
{
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, 2);
	exit(1);
}

//==============================================================================
void App::handleLoggerMessages(const Logger::Info& info)
{
	std::ostream* out = NULL;
	const char* x = NULL;
	const char* terminalColor;

	switch(info.type)
	{
	case Logger::LMT_NORMAL:
		out = &std::cout;
		x = "Info";
		terminalColor = "\033[0;32m";
		break;
	case Logger::LMT_ERROR:
		out = &std::cerr;
		x = "Error";
		terminalColor = "\033[0;31m";
		break;
	case Logger::LMT_WARNING:
		out = &std::cerr;
		x = "Warn";
		terminalColor = "\033[0;33m";
		break;
	}

	(*out) << terminalColor << "(" << info.file << ":" << info.line << " "
		<< info.func << ") " << x << ": " << info.msg << "\033[0m" << std::endl;
}

//==============================================================================
void App::parseCommandLineArgs(int argc, char* argv[])
{
#if 0
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
#endif
}

//==============================================================================
void App::init(int argc, char* argv[])
{
	// Install signal handlers
	signal(SIGSEGV, handler);
	signal(SIGBUS, handler);
	signal(SIGFPE, handler);

	// send output to handleMessageHanlderMsgs
	ANKI_CONNECT(&LoggerSingleton::get(), messageRecieved, 
		this, handleLoggerMessages);

	parseCommandLineArgs(argc, argv);
	printAppInfo();
	initDirs();

	timerTick = 1.0 / 60.0; // in sec. 1.0 / period
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
void App::quit(int code)
{
#if 0
	SDL_FreeSurface(iconImage);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(windowId);
	SDL_Quit();
	exit(code);
#endif
}

//==============================================================================
void App::printAppInfo()
{
	std::stringstream msg;
	msg << "App info: ";
	msg << "AnKi " << ANKI_VERSION_MAJOR << "." << ANKI_VERSION_MINOR << ", ";
#if !ANKI_DEBUG
	msg << "Release";
#else
	msg << "Debug";
#endif
	msg << " build,";

	msg << " " << ANKI_CPU_ARCH_STR;
	msg << " " << ANKI_GL_STR;
	msg << " " << ANKI_WINDOW_BACKEND_STR;
	msg << " CPU cores " << getCpuCoresCount();

	msg << " build date " __DATE__ ", " << "rev " << ANKI_REVISION;

	ANKI_LOGI(msg.str());
}

} // end namespace anki

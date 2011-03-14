#ifndef APP_H
#define APP_H

#include <SDL/SDL.h>
#include <boost/filesystem.hpp>
#include "StdTypes.h"
#include "Properties.h"
#include "Exception.h"
#include "Singleton.h"


class StdinListener;
class Scene;
class Camera;
class Input;


/// This class holds all the global objects of the application and its also responsible for some of the SDL stuff.
/// It should be singleton
class App
{
	PROPERTY_R(uint, windowW, getWindowWidth) ///< The main window width
	PROPERTY_R(uint, windowH, getWindowHeight) ///< The main window height
	PROPERTY_R(boost::filesystem::path, settingsPath, getSettingsPath)
	PROPERTY_R(boost::filesystem::path, cachePath, getCachePath)

	public:
		App() {}
		~App() {}
		void init(int argc, char* argv[]);
		void quit(int code);
		void waitForNextFrame();
		void togleFullScreen();
		void swapBuffers();

		/// The func pools the stdinListener for string in the console, if there are any it executes them with
		/// scriptingEngine
		void execStdinScpripts();

		static void printAppInfo();

		uint getDesktopWidth() const;
		uint getDesktopHeight() const;

		/// @name Accessors
		/// @{
		bool isTerminalColoringEnabled() const;

		Camera* getActiveCam() {return activeCam;}
		void setActiveCam(Camera* cam) {activeCam = cam;}

		GETTER_SETTER(uint, timerTick, getTimerTick, setTimerTick)
		/// @}

		/// @return Returns the number of milliseconds since SDL library initialization
		static uint getTicks();

	private:
		uint timerTick;
		bool terminalColoringEnabled; ///< Terminal coloring for Unix terminals. Default on
		uint time;
		SDL_WindowID windowId;
		SDL_GLContext glContext;
		SDL_Surface* iconImage;
		bool fullScreenFlag;
		Camera* activeCam; ///< Pointer to the current camera

		void parseCommandLineArgs(int argc, char* argv[]);

		/// A slot to handle the messageHandler's signal
		void handleMessageHanlderMsgs(const char* file, int line, const char* func, const char* msg);

		void initWindow();
		void initDirs();
		void initRenderer();
};


inline bool App::isTerminalColoringEnabled() const
{
	return terminalColoringEnabled;
}


typedef Singleton<App> AppSingleton;


#endif

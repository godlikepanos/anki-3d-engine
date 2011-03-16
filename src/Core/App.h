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


/// The core class of the engine.
///
/// - It initializes the window
/// - it holds the global state and thus it parses the command line arguments
/// - It manipulates the main loop timer
class App
{
	public:
		App() {}
		~App() {}

		/// This method:
		/// - Initialize the window
		/// - Initialize the main renderer
		/// - Initialize and start the stdin listener
		/// - Initialize the scripting engine
		void init(int argc, char* argv[]);

		/// What it does:
		/// - Destroy the window
		/// - call exit()
		void quit(int code);

		/// Self explanatory
		void togleFullScreen();

		/// Wrapper for an SDL function that swaps the buffers
		void swapBuffers();

		/// The func pools the stdinListener for string in the console, if there are any it executes them with
		/// scriptingEngine
		void execStdinScpripts();

		static void printAppInfo();

		uint getDesktopWidth() const;
		uint getDesktopHeight() const;

		/// @name Accessors
		/// @{
		Camera* getActiveCam() {return activeCam;}
		void setActiveCam(Camera* cam) {activeCam = cam;}

		GETTER_SETTER(uint, timerTick, getTimerTick, setTimerTick)
		GETTER_R_BY_VAL(bool, terminalColoringEnabled, isTerminalColoringEnabled)
		GETTER_R_BY_VAL(uint, windowW, getWindowWidth)
		GETTER_R(uint, windowH, getWindowHeight)
		GETTER_R(boost::filesystem::path, settingsPath, getSettingsPath)
		GETTER_R(boost::filesystem::path, cachePath, getCachePath)
		/// @}

	private:
		uint windowW; ///< The main window width
		uint windowH; ///< The main window height
		boost::filesystem::path settingsPath; ///< The path that holds the configuration
		boost::filesystem::path cachePath; ///< This is used as a cache
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


typedef Singleton<App> AppSingleton;


#endif

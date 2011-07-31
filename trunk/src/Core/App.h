#ifndef APP_H
#define APP_H

#include <SDL/SDL.h>
#include <boost/filesystem.hpp>
#include "Util/StdTypes.h"
#include "Util/Accessors.h"


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

		static void printAppInfo();

		uint getDesktopWidth() const;
		uint getDesktopHeight() const;

		/// @name Accessors
		/// @{
		Camera* getActiveCam() {return activeCam;}
		void setActiveCam(Camera* cam) {activeCam = cam;}

		GETTER_SETTER(float, timerTick, getTimerTick, setTimerTick)
		GETTER_R_BY_VAL(bool, terminalColoringEnabled,
			isTerminalColoringEnabled)
		GETTER_R_BY_VAL(uint, windowW, getWindowWidth)
		GETTER_R(uint, windowH, getWindowHeight)
		GETTER_R(boost::filesystem::path, settingsPath, getSettingsPath)
		GETTER_R(boost::filesystem::path, cachePath, getCachePath)
		GETTER_RW(SDL_WindowID, windowId, getWindowId)
		/// @}

	private:
		uint windowW; ///< The main window width
		uint windowH; ///< The main window height
		/// The path that holds the configuration
		boost::filesystem::path settingsPath;
		/// This is used as a cache
		boost::filesystem::path cachePath;
		float timerTick;
		/// Terminal coloring for Unix terminals. Default on
		bool terminalColoringEnabled;
		SDL_WindowID windowId;
		SDL_GLContext glContext;
		SDL_Surface* iconImage;
		bool fullScreenFlag;
		Camera* activeCam; ///< Pointer to the current camera

		void parseCommandLineArgs(int argc, char* argv[]);

		/// A slot to handle the messageHandler's signal
		void handleMessageHanlderMsgs(const char* file, int line,
			const char* func, const char* msg);

		void initWindow();
		void initDirs();
		void initRenderer();
};


#endif

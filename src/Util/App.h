#ifndef _APP_H_
#define _APP_H_

#include <SDL.h>
#include <boost/filesystem.hpp>
#include "Common.h"


/**
 * This class holds all the global objects of the application and its also responsible for some of the SDL stuff.
 * It should be singleton
 */
class App
{
	PROPERTY_R(uint, windowW, getWindowWidth) ///< The main window width
	PROPERTY_R(uint, windowH, getWindowHeight) ///< The main window height
	PROPERTY_R(bool, terminalColoringEnabled, isTerminalColoringEnabled) ///< Terminal coloring for Unix terminals. Default on
	PROPERTY_R(filesystem::path, settingsPath, getSettingsPath)
	PROPERTY_R(filesystem::path, cachePath, getCachePath)

	PROPERTY_RW(class Scene*, scene, setScene, getScene) ///< Pointer to the current scene
	PROPERTY_RW(class MainRenderer*, mainRenderer, setMainRenderer, getMainRenderer) ///< Pointer to the main renderer
	PROPERTY_RW(class Camera*, activeCam, setActiveCam, getActiveCam) ///< Pointer to the current camera

	private:
		static bool isCreated; ///< A flag to ensure one @ref App instance
		uint time;
		SDL_WindowID windowId;
		SDL_GLContext glContext;
		SDL_Surface* iconImage;
		bool fullScreenFlag;

		void parseCommandLineArgs(int argc, char* argv[]);

	public:
		uint timerTick;

		App(int argc, char* argv[]);
		~App() {}
		void initWindow();
		void quit(int code);
		void waitForNextFrame();
		void togleFullScreen();
		void swapBuffers();
		static void printAppInfo();

		uint getDesktopWidth() const;
		uint getDesktopHeight() const;

		/**
		 * @return Returns the number of milliseconds since SDL library initialization
		 */
		static uint getTicks();
};


#endif

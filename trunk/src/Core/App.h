#ifndef APP_H
#define APP_H

#include <SDL/SDL.h>
#include <boost/filesystem.hpp>
#include "Common.h"
#include "Object.h"


class ScriptingEngine;


/**
 * This class holds all the global objects of the application and its also responsible for some of the SDL stuff.
 * It should be singleton
 */
class App: public Object
{
	PROPERTY_R(uint, windowW, getWindowWidth) ///< The main window width
	PROPERTY_R(uint, windowH, getWindowHeight) ///< The main window height
	//PROPERTY_R(bool, terminalColoringEnabled, isTerminalColoringEnabled)
	PROPERTY_R(filesystem::path, settingsPath, getSettingsPath)
	PROPERTY_R(filesystem::path, cachePath, getCachePath)

	//PROPERTY_RW(class Scene*, scene, setScene, getScene) ///< Pointer to the current scene
	PROPERTY_RW(class MainRenderer*, mainRenderer, setMainRenderer, getMainRenderer) ///< Pointer to the main renderer
	PROPERTY_RW(class Camera*, activeCam, setActiveCam, getActiveCam) ///< Pointer to the current camera

	private:
		static bool isCreated; ///< A flag to ensure one @ref App instance
		bool terminalColoringEnabled; ///< Terminal coloring for Unix terminals. Default on
		class Scene* scene;
		ScriptingEngine* scriptingEngine;
		uint time;
		SDL_WindowID windowId;
		SDL_GLContext glContext;
		SDL_Surface* iconImage;
		bool fullScreenFlag;

		void parseCommandLineArgs(int argc, char* argv[]);

	public:
		uint timerTick;

		App(int argc, char* argv[], Object* parent = NULL);
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
		 * @name Accessors
		 */
		/**@{*/
		bool isTerminalColoringEnabled() const;
		Scene* getScene();
		ScriptingEngine& getScriptingEngine();
		/**@}*/

		/**
		 * @return Returns the number of milliseconds since SDL library initialization
		 */
		static uint getTicks();
};


inline bool App::isTerminalColoringEnabled() const
{
	return terminalColoringEnabled;
}


inline Scene* App::getScene()
{
	DEBUG_ERR(scene == NULL);
	return scene;
}


inline ScriptingEngine& App::getScriptingEngine()
{
	DEBUG_ERR(scriptingEngine == NULL);
	return *scriptingEngine;
}


#endif

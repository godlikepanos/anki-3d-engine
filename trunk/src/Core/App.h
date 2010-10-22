#ifndef APP_H
#define APP_H

#include <SDL/SDL.h>
#include <boost/filesystem.hpp>
#include "Object.h"
#include "MessageHandler.h"
#include "StdTypes.h"
#include "Properties.h"
#include "Exception.h"


class ScriptingEngine;
class StdinListener;
class Scene;
class MainRenderer;
class Camera;
class Input;


/// The one and only global variable
extern class App* app;


/// This class holds all the global objects of the application and its also responsible for some of the SDL stuff.
/// It should be singleton
class App: public Object
{
	PROPERTY_R(uint, windowW, getWindowWidth) ///< The main window width
	PROPERTY_R(uint, windowH, getWindowHeight) ///< The main window height
	PROPERTY_R(boost::filesystem::path, settingsPath, getSettingsPath)
	PROPERTY_R(boost::filesystem::path, cachePath, getCachePath)

	public:
		uint timerTick;

		App(int argc, char* argv[], Object* parent = NULL);
		~App() {}
		void initWindow();
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
		Scene& getScene();
		ScriptingEngine& getScriptingEngine();
		StdinListener& getStdinLintener();
		MainRenderer& getMainRenderer();
		Camera* getActiveCam() {return activeCam;}
		void setActiveCam(Camera* cam) {activeCam = cam;}
		MessageHandler& getMessageHandler();
		Input& getInput();
		/// @}

		/// @return Returns the number of milliseconds since SDL library initialization
		static uint getTicks();

	private:
		static bool isCreated; ///< A flag to ensure one @ref App instance
		bool terminalColoringEnabled; ///< Terminal coloring for Unix terminals. Default on
		uint time;
		SDL_WindowID windowId;
		SDL_GLContext glContext;
		SDL_Surface* iconImage;
		bool fullScreenFlag;
		Camera* activeCam; ///< Pointer to the current camera

		/// @name Pointers to serious subsystems
		/// @{
		Scene* scene;
		ScriptingEngine* scriptingEngine;
		MainRenderer* mainRenderer;
		StdinListener* stdinListener;
		MessageHandler* messageHandler;
		Input* input;
		/// @}

		void parseCommandLineArgs(int argc, char* argv[]);

		/// A slot to handle the messageHandler's signal
		void handleMessageHanlderMsgs(const char* file, int line, const char* func, const std::string& msg);
};


inline bool App::isTerminalColoringEnabled() const
{
	return terminalColoringEnabled;
}


inline Scene& App::getScene()
{
	RASSERT_THROW_EXCEPTION(scene == NULL);
	return *scene;
}


inline ScriptingEngine& App::getScriptingEngine()
{
	RASSERT_THROW_EXCEPTION(scriptingEngine == NULL);
	return *scriptingEngine;
}


inline StdinListener& App::getStdinLintener()
{
	RASSERT_THROW_EXCEPTION(stdinListener == NULL);
	return *stdinListener;
}


inline MainRenderer& App::getMainRenderer()
{
	RASSERT_THROW_EXCEPTION(mainRenderer == NULL);
	return *mainRenderer;
}


inline MessageHandler& App::getMessageHandler()
{
	RASSERT_THROW_EXCEPTION(messageHandler == NULL);
	return *messageHandler;
}


inline Input& App::getInput()
{
	RASSERT_THROW_EXCEPTION(input == NULL);
	return *input;
}


#endif

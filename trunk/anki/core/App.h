#ifndef ANKI_CORE_APP_H
#define ANKI_CORE_APP_H

#include <SDL/SDL.h>
#include <boost/filesystem.hpp>
#include "anki/util/StdTypes.h"
#include "anki/core/Logger.h"


namespace anki {


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
		Camera* getActiveCam()
		{
			return activeCam;
		}
		void setActiveCam(Camera* cam)
		{
			activeCam = cam;
		}

		float getTimerTick() const
		{
			return timerTick;
		}
		float& getTimerTick()
		{
			return timerTick;
		}
		void setTimerTick(const float x)
		{
			timerTick = x;
		}

		uint getWindowWidth() const
		{
			return windowW;
		}

		uint getWindowHeight() const
		{
			return windowH;
		}

		const boost::filesystem::path& getSettingsPath() const
		{
			return settingsPath;
		}

		const boost::filesystem::path& getCachePath() const
		{
			return cachePath;
		}
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
			const char* func, Logger::MessageType, const char* msg);

		void initWindow();
		void initDirs();
		void initRenderer();
};


} // end namespace


#endif

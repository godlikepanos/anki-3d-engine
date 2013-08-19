#ifndef ANKI_CORE_APP_H
#define ANKI_CORE_APP_H

#include "anki/core/Logger.h"
#include "anki/util/Singleton.h"
#include "anki/util/StringList.h"

namespace anki {

class StdinListener;
class SceneGraph;
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
	ANKI_HAS_SLOTS(App)

	App()
	{}
	~App()
	{}

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

	static void printAppInfo();

	/// @name Accessors
	/// @{
	F32 getTimerTick() const
	{
		return timerTick;
	}
	F32& getTimerTick()
	{
		return timerTick;
	}
	void setTimerTick(const F32 x)
	{
		timerTick = x;
	}

	const std::string& getSettingsPath() const
	{
		return settingsPath;
	}

	const std::string& getCachePath() const
	{
		return cachePath;
	}

	const StringList& getDataPaths() const
	{
		return dataPaths;
	}
	/// @}

private:
	/// The path that holds the configuration
	std::string settingsPath;
	/// This is used as a cache
	std::string cachePath;
	StringList dataPaths;
	F32 timerTick;

	void parseCommandLineArgs(int argc, char* argv[]);

	/// A slot to handle the messageHandler's signal
	void handleLoggerMessages(const Logger::Info& info);
	ANKI_SLOT(handleLoggerMessages, const Logger::Info&)

	void initWindow();
	void initDirs();
	void initRenderer();
};

typedef Singleton<App> AppSingleton;

} // end namespace

#endif

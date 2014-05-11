#ifndef ANKI_CORE_APP_H
#define ANKI_CORE_APP_H

#include "anki/Config.h"
#include "anki/util/Singleton.h"
#include "anki/util/StringList.h"
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki {

#if ANKI_OS == ANKI_OS_ANDROID
extern android_app* gAndroidApp;
#endif

/// The core class of the engine.
///
/// - It initializes the window
/// - it holds the global state and thus it parses the command line arguments
/// - It manipulates the main loop timer
class App
{
public:
	App()
	{}
	~App()
	{}

	/// Initialize the app
	void init();

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

	const String& getSettingsPath() const
	{
		return settingsPath;
	}

	const String& getCachePath() const
	{
		return cachePath;
	}
	/// @}

	/// What it does:
	/// - Destroy the window
	/// - call exit()
	void quit(int code);

	/// Run the main loop
	void mainLoop();

	static void printAppInfo();

private:
	/// The path that holds the configuration
	String settingsPath;
	/// This is used as a cache
	String cachePath;
	F32 timerTick;

	void initDirs();
};

typedef Singleton<App> AppSingleton;

} // end namespace anki

#endif

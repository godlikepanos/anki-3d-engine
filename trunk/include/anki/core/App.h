#ifndef ANKI_CORE_APP_H
#define ANKI_CORE_APP_H

#include "anki/Config.h"
#include "anki/util/Singleton.h"
#include "anki/util/StringList.h"
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki {

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
	void init(void* systemSpecificData);

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

#if ANKI_OS == ANKI_OS_ANDROID
	android_app& getAndroidApp()
	{
		ANKI_ASSERT(andApp);
		return *andApp;
	}
#endif
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
	std::string settingsPath;
	/// This is used as a cache
	std::string cachePath;
	F32 timerTick;

#if ANKI_OS == ANKI_OS_ANDROID
	android_app* andApp = nullptr;
#endif

	void initDirs();
};

typedef Singleton<App> AppSingleton;

} // end namespace anki

#endif

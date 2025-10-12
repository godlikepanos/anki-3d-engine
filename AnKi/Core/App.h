// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/Functions.h>

namespace anki {

ANKI_CVAR(NumericCVar<U32>, Window, Width, 1920, 16, 16 * 1024, "Width")
ANKI_CVAR(NumericCVar<U32>, Window, Height, 1080, 16, 16 * 1024, "Height")
ANKI_CVAR(NumericCVar<U32>, Window, Fullscreen, 1, 0, 2, "0: windowed, 1: borderless fullscreen, 2: exclusive fullscreen")
ANKI_CVAR(BoolCVar, Window, Maximized, false, "Maximize")
ANKI_CVAR(BoolCVar, Window, Borderless, false, "Borderless")
ANKI_CVAR(NumericCVar<U32>, Core, TargetFps, 60u, 1u, kMaxU32, "Target FPS")
ANKI_CVAR(NumericCVar<U32>, Core, JobThreadCount, clamp(getCpuCoresCount() / 2u, 2u, 16u), 2u, 1024u, "Number of job thread")
ANKI_CVAR(NumericCVar<U32>, Core, DisplayStats, 0, 0, 2, "Display stats, 0: None, 1: Simple, 2: Detailed")
ANKI_CVAR(BoolCVar, Core, ClearCaches, false, "Clear all caches")
ANKI_CVAR(BoolCVar, Core, VerboseLog, false, "Verbose logging")
ANKI_CVAR(BoolCVar, Core, BenchmarkMode, false, "Run in a benchmark mode. Fixed timestep, unlimited target FPS")
ANKI_CVAR(NumericCVar<U32>, Core, BenchmarkModeFrameCount, 60 * 60 * 2, 1, kMaxU32, "How many frames the benchmark will run before it quits")
ANKI_CVAR(BoolCVar, Core, MeshletRendering, false, "Do meshlet culling and rendering")

#if ANKI_PLATFORM_MOBILE
ANKI_CVAR(BoolCVar, Core, MaliHwCounters, false, "Enable Mali counters")
#endif

ANKI_SVAR(CpuTotalTime, StatCategory::kTime, "CPU total", StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates)

/// The core class of the engine.
class App
{
public:
	App(CString applicationName, AllocAlignedCallback allocCb = allocAligned, void* allocCbUserData = nullptr);

	virtual ~App();

	CString getSettingsDirectory() const
	{
		return m_settingsDir;
	}

	CString getCacheDirectory() const
	{
		return m_cacheDir;
	}

	/// Run the main loop.
	Error mainLoop();

	/// User defined init code that will execute before all subsystems have initialized. Will be executed just before the main loop. Useful for
	/// setting cvars.
	virtual Error userPreInit()
	{
		// Do nothing
		return Error::kNone;
	}

	/// User defined init code that will execute after all subsystems have initialized. Will be executed just before the main loop and after
	/// everything has been initialized.
	virtual Error userPostInit()
	{
		// Do nothing
		return Error::kNone;
	}

	/// User defined code to run along with the other main loop code.
	virtual Error userMainLoop([[maybe_unused]] Bool& quit, [[maybe_unused]] Second elapsedTime)
	{
		// Do nothing
		return Error::kNone;
	}

	Bool toggleDeveloperConsole();

	Bool getDeveloperConsoleEnabled() const
	{
		return m_consoleEnabled;
	}

	CString getApplicationName() const
	{
		return m_appName;
	}

private:
	Bool m_consoleEnabled = false;
	CoreString m_settingsDir; ///< The path that holds the configuration
	CoreString m_cacheDir; ///< This is used as a cache
	CoreString m_appName;

	void* m_originalAllocUserData = nullptr;
	AllocAlignedCallback m_originalAllocCallback = nullptr;
	void* m_allocUserData = nullptr;
	AllocAlignedCallback m_allocCallback = nullptr;

	static void* statsAllocCallback(void* userData, void* ptr, PtrSize size, PtrSize alignment);

	Error init();

	Error initDirs();
	void cleanup();
};

} // end namespace anki

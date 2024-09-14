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
#include <AnKi/Ui/UiImmediateModeBuilder.h>

namespace anki {

inline NumericCVar<U32> g_windowWidthCVar("Core", "Width", 1920, 16, 16 * 1024, "Width");
inline NumericCVar<U32> g_windowHeightCVar("Core", "Height", 1080, 16, 16 * 1024, "Height");
inline NumericCVar<U32> g_windowFullscreenCVar("Core", "WindowFullscreen", 1, 0, 2, "0: windowed, 1: borderless fullscreen, 2: exclusive fullscreen");
inline NumericCVar<U32> g_targetFpsCVar("Core", "TargetFps", 60u, 1u, kMaxU32, "Target FPS");
inline NumericCVar<U32> g_jobThreadCountCVar("Core", "JobThreadCount", clamp(getCpuCoresCount() / 2u, 2u, 16u), 2u, 1024u, "Number of job thread");
inline NumericCVar<U32> g_displayStatsCVar("Core", "DisplayStats", 0, 0, 2, "Display stats, 0: None, 1: Simple, 2: Detailed");
inline BoolCVar g_clearCachesCVar("Core", "ClearCaches", false, "Clear all caches");
inline BoolCVar g_verboseLogCVar("Core", "VerboseLog", false, "Verbose logging");
inline BoolCVar g_benchmarkModeCVar("Core", "BenchmarkMode", false, "Run in a benchmark mode. Fixed timestep, unlimited target FPS");
inline NumericCVar<U32> g_benchmarkModeFrameCountCVar("Core", "BenchmarkModeFrameCount", 60 * 60 * 2, 1, kMaxU32,
													  "How many frames the benchmark will run before it quits");
inline BoolCVar g_meshletRenderingCVar("Core", "MeshletRendering", false, "Do meshlet culling and rendering");

#if ANKI_PLATFORM_MOBILE
inline BoolCVar g_maliHwCountersCVar("Core", "MaliHwCounters", false, "Enable Mali counters");
#endif

inline StatCounter g_cpuTotalTimeStatVar(StatCategory::kTime, "CPU total",
										 StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates);

/// The core class of the engine.
class App
{
public:
	App(AllocAlignedCallback allocCb = allocAligned, void* allocCbUserData = nullptr);

	virtual ~App();

	/// Initialize the application.
	Error init();

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

	/// The user code to run along with the other main loop code.
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

private:
	Bool m_consoleEnabled = false;
	CoreString m_settingsDir; ///< The path that holds the configuration
	CoreString m_cacheDir; ///< This is used as a cache

	void* m_originalAllocUserData = nullptr;
	AllocAlignedCallback m_originalAllocCallback = nullptr;

	static void* statsAllocCallback(void* userData, void* ptr, PtrSize size, PtrSize alignment);

	void initMemoryCallbacks(AllocAlignedCallback& allocCb, void*& allocCbUserData);

	Error initInternal();

	Error initDirs();
	void cleanup();
};

} // end namespace anki

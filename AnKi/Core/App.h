// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/CVarSet.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Ui/UiImmediateModeBuilder.h>

namespace anki {

// Forward
class UiQueueElement;
class RenderQueue;
class StatCounter;
extern NumericCVar<U32> g_windowWidthCVar;
extern NumericCVar<U32> g_windowHeightCVar;
extern NumericCVar<U32> g_windowFullscreenCVar;
extern NumericCVar<U32> g_targetFpsCVar;
extern NumericCVar<U32> g_displayStatsCVar;
extern StatCounter g_cpuTotalTimeStatVar;
extern StatCounter g_rendererGpuTimeStatVar;

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

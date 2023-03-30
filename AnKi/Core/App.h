// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Ui/UiImmediateModeBuilder.h>

namespace anki {

// Forward
class UiQueueElement;
class RenderQueue;

/// The core class of the engine.
class App
{
public:
	App();
	virtual ~App();

	/// Initialize the application.
	Error init(AllocAlignedCallback allocCb, void* allocCbUserData);

	const String& getSettingsDirectory() const
	{
		return m_settingsDir;
	}

	const String& getCacheDirectory() const
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

	void setDisplayDeveloperConsole(Bool display)
	{
		m_consoleEnabled = display;
	}

	Bool getDisplayDeveloperConsole() const
	{
		return m_consoleEnabled;
	}

private:
	// Misc
	UiImmediateModeBuilderPtr m_statsUi;
	UiImmediateModeBuilderPtr m_console;
	Bool m_consoleEnabled = false;
	CoreString m_settingsDir; ///< The path that holds the configuration
	CoreString m_cacheDir; ///< This is used as a cache
	U64 m_resourceCompletedAsyncTaskCount = 0;

	class MemStats
	{
	public:
		Atomic<PtrSize> m_allocatedMem = {0};
		Atomic<U64> m_allocCount = {0};
		Atomic<U64> m_freeCount = {0};

		void* m_originalUserData = nullptr;
		AllocAlignedCallback m_originalAllocCallback = nullptr;

		static void* allocCallback(void* userData, void* ptr, PtrSize size, PtrSize alignment);
	} m_memStats;

	void initMemoryCallbacks(AllocAlignedCallback& allocCb, void*& allocCbUserData);

	Error initInternal(AllocAlignedCallback allocCb, void* allocCbUserData);

	Error initDirs();
	void cleanup();

	/// Inject a new UI element in the render queue for displaying various stuff.
	void injectUiElements(CoreDynamicArray<UiQueueElement>& elements, RenderQueue& rqueue);

	void setSignalHandlers();
};

} // end namespace anki

// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
class CoreTracer;
class ConfigSet;
class ThreadHive;
class NativeWindow;
class Input;
class GrManager;
class MainRenderer;
class PhysicsWorld;
class SceneGraph;
class ScriptManager;
class ResourceManager;
class ResourceFilesystem;
class StagingGpuMemoryPool;
class UnifiedGeometryMemoryPool;
class UiManager;
class UiQueueElement;
class RenderQueue;
class MaliHwCounters;

/// The core class of the engine.
class App
{
public:
	App();
	virtual ~App();

	/// Initialize the application.
	/// @param[in,out] config The config. Needs to be alive as long as the app is alive.
	Error init(ConfigSet* config, AllocAlignedCallback allocCb, void* allocCbUserData);

	const String& getSettingsDirectory() const
	{
		return m_settingsDir;
	}

	const String& getCacheDirectory() const
	{
		return m_cacheDir;
	}

	ThreadHive& getThreadHive()
	{
		return *m_threadHive;
	}

	HeapMemoryPool& getMemoryPool()
	{
		return m_mainPool;
	}

	Timestamp getGlobalTimestamp() const
	{
		return m_globalTimestamp;
	}

	/// Run the main loop.
	Error mainLoop();

	/// The user code to run along with the other main loop code.
	virtual Error userMainLoop([[maybe_unused]] Bool& quit, [[maybe_unused]] Second elapsedTime)
	{
		// Do nothing
		return Error::kNone;
	}

	const ConfigSet& getConfig() const
	{
		return *m_config;
	}

	ConfigSet& getConfig()
	{
		return *m_config;
	}

	Input& getInput()
	{
		return *m_input;
	}

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	MainRenderer& getMainRenderer()
	{
		return *m_renderer;
	}

	ResourceManager& getResourceManager()
	{
		return *m_resources;
	}

	ScriptManager& getScriptManager()
	{
		return *m_script;
	}

	PhysicsWorld& getPhysicsWorld()
	{
		return *m_physics;
	}

	NativeWindow& getWindow()
	{
		return *m_window;
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
	HeapMemoryPool m_mainPool;

	// Sybsystems
	ConfigSet* m_config = nullptr;
#if ANKI_ENABLE_TRACE
	CoreTracer* m_coreTracer = nullptr;
#endif
	NativeWindow* m_window = nullptr;
	Input* m_input = nullptr;
	ThreadHive* m_threadHive = nullptr;
	GrManager* m_gr = nullptr;
	MaliHwCounters* m_maliHwCounters = nullptr;
	UnifiedGeometryMemoryPool* m_unifiedGometryMemPool = nullptr;
	StagingGpuMemoryPool* m_stagingMem = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_resourceFs = nullptr;
	ResourceManager* m_resources = nullptr;
	UiManager* m_ui = nullptr;
	MainRenderer* m_renderer = nullptr;
	SceneGraph* m_scene = nullptr;
	ScriptManager* m_script = nullptr;

	// Misc
	UiImmediateModeBuilderPtr m_statsUi;
	UiImmediateModeBuilderPtr m_console;
	Bool m_consoleEnabled = false;
	Timestamp m_globalTimestamp = 1;
	String m_settingsDir; ///< The path that holds the configuration
	String m_cacheDir; ///< This is used as a cache
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
	void injectUiElements(DynamicArrayRaii<UiQueueElement>& elements, RenderQueue& rqueue);

	void setSignalHandlers();
};

} // end namespace anki

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/Allocator.h>
#include <anki/util/String.h>
#include <anki/util/Ptr.h>
#include <anki/core/Timestamp.h>
#if ANKI_OS == ANKI_OS_ANDROID
#include <android_native_app_glue.h>
#endif

namespace anki
{

#if ANKI_OS == ANKI_OS_ANDROID
extern android_app* gAndroidApp;
#endif

// Forward
class ConfigSet;
class ThreadPool;
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
class StagingGpuMemoryManager;

/// The core class of the engine.
class App
{
public:
	App();
	virtual ~App();

	ANKI_USE_RESULT Error init(const ConfigSet& config, AllocAlignedCallback allocCb, void* allocCbUserData);

	F32 getTimerTick() const
	{
		return m_timerTick;
	}

	void setTimerTick(const F32 x)
	{
		m_timerTick = x;
	}

	const String& getSettingsDirectory() const
	{
		return m_settingsDir;
	}

	const String& getCacheDirectory() const
	{
		return m_cacheDir;
	}

	AllocAlignedCallback getAllocationCallback() const
	{
		return m_allocCb;
	}

	void* getAllocationCallbackData() const
	{
		return m_allocCbData;
	}

	ThreadPool& getThreadPool()
	{
		return *m_threadpool;
	}

	ThreadHive& getThreadHive()
	{
		return *m_threadHive;
	}

	HeapAllocator<U8>& getAllocator()
	{
		return m_heapAlloc;
	}

	Timestamp getGlobalTimestamp() const
	{
		return m_globalTimestamp;
	}

	/// Run the main loop.
	ANKI_USE_RESULT Error mainLoop();

	/// The user code to run along with the other main loop code.
	virtual ANKI_USE_RESULT Error userMainLoop(Bool& quit)
	{
		// Do nothing
		return ErrorCode::NONE;
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

	HeapAllocator<U8> getAllocator() const
	{
		return m_heapAlloc;
	}

private:
	// Allocation
	AllocAlignedCallback m_allocCb;
	void* m_allocCbData;
	HeapAllocator<U8> m_heapAlloc;

	// Sybsystems
	NativeWindow* m_window = nullptr;
	Input* m_input = nullptr;
	GrManager* m_gr = nullptr;
	StagingGpuMemoryManager* m_stagingMem = nullptr;
	PhysicsWorld* m_physics = nullptr;
	ResourceFilesystem* m_resourceFs = nullptr;
	ResourceManager* m_resources = nullptr;
	MainRenderer* m_renderer = nullptr;
	SceneGraph* m_scene = nullptr;
	ScriptManager* m_script = nullptr;

	// Misc
	Timestamp m_globalTimestamp = 0;
	ThreadPool* m_threadpool = nullptr;
	ThreadHive* m_threadHive = nullptr;
	String m_settingsDir; ///< The path that holds the configuration
	String m_cacheDir; ///< This is used as a cache
	F32 m_timerTick;
	U64 m_resourceCompletedAsyncTaskCount = 0;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& config, AllocAlignedCallback allocCb, void* allocCbUserData);

	ANKI_USE_RESULT Error initDirs();
	void cleanup();
};

} // end namespace anki

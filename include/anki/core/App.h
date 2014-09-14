// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_APP_H
#define ANKI_CORE_APP_H

#include "anki/util/Allocator.h"
#include "anki/util/String.h"
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki {

#if ANKI_OS == ANKI_OS_ANDROID
extern android_app* gAndroidApp;
#endif

// Forward
class ConfigSet;
class Threadpool;
class NativeWindow;
class Input;
class GlDevice;
class MainRenderer;
class SceneGraph;
class ScriptManager;
class ResourceManager;

/// The core class of the engine.
class App
{
public:
	using UserMainLoopCallback = void(*)(App& app, void* userData);

	/// Create and initialize the application
	App(const ConfigSet& config, 
		AllocAlignedCallback allocCb, void* allocCbUserData);

	~App()
	{
		// TODO
	}

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

	Threadpool& getThreadpool()
	{
		return *m_threadpool;
	}

	HeapAllocator<U8>& getAllocator()
	{
		return m_heapAlloc;
	}

	/// TODO
	void quit(int code);

	/// Run the main loop
	void mainLoop(UserMainLoopCallback callback, void* userData);

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

private:
	// Allocation
	AllocAlignedCallback m_allocCb;
	void* m_allocCbData;
	HeapAllocator<U8> m_heapAlloc;

	// Sybsystems
	NativeWindow* m_window = nullptr;
	Input* m_input = nullptr;
	GlDevice* m_gl = nullptr;
	ResourceManager* m_resources = nullptr;
	MainRenderer* m_renderer = nullptr;
	SceneGraph* m_scene = nullptr;
	ScriptManager* m_script = nullptr;

	// Misc
	void* m_ctx = nullptr;
	Threadpool* m_threadpool = nullptr;
	String m_settingsDir; ///< The path that holds the configuration
	String m_cacheDir; ///< This is used as a cache
	F32 m_timerTick;

	/// Initialize the app
	void init(const ConfigSet& config);

	void initDirs();

	void printAppInfo();

	static void makeCurrent(void* data);
	static void swapWindow(void* window);
};

} // end namespace anki

#endif

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/App.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/util/System.h>
#include <anki/util/ThreadPool.h>
#include <anki/util/ThreadHive.h>
#include <anki/core/Trace.h>

#include <anki/core/NativeWindow.h>
#include <anki/input/Input.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ResourceManager.h>
#include <anki/physics/PhysicsWorld.h>
#include <anki/renderer/MainRenderer.h>
#include <anki/script/ScriptManager.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/resource/AsyncLoader.h>
#include <anki/core/StagingGpuMemoryManager.h>

#if ANKI_OS == ANKI_OS_ANDROID
#include <android_native_app_glue.h>
#endif

namespace anki
{

#if ANKI_OS == ANKI_OS_ANDROID
/// The one and only android hack
android_app* gAndroidApp = nullptr;
#endif

App::App()
{
}

App::~App()
{
	cleanup();
}

void App::cleanup()
{
	if(m_script)
	{
		m_heapAlloc.deleteInstance(m_script);
		m_script = nullptr;
	}

	if(m_scene)
	{
		m_heapAlloc.deleteInstance(m_scene);
		m_scene = nullptr;
	}

	if(m_renderer)
	{
		m_heapAlloc.deleteInstance(m_renderer);
		m_renderer = nullptr;
	}

	if(m_resources)
	{
		m_heapAlloc.deleteInstance(m_resources);
		m_resources = nullptr;
	}

	if(m_resourceFs)
	{
		m_heapAlloc.deleteInstance(m_resourceFs);
		m_resourceFs = nullptr;
	}

	if(m_physics)
	{
		m_heapAlloc.deleteInstance(m_physics);
		m_physics = nullptr;
	}

	if(m_stagingMem)
	{
		m_heapAlloc.deleteInstance(m_stagingMem);
		m_stagingMem = nullptr;
	}

	if(m_gr)
	{
		m_heapAlloc.deleteInstance(m_gr);
		m_gr = nullptr;
	}

	if(m_threadpool)
	{
		m_heapAlloc.deleteInstance(m_threadpool);
		m_threadpool = nullptr;
	}

	if(m_threadHive)
	{
		m_heapAlloc.deleteInstance(m_threadHive);
		m_threadHive = nullptr;
	}

	if(m_input)
	{
		m_heapAlloc.deleteInstance(m_input);
		m_input = nullptr;
	}

	if(m_window)
	{
		m_heapAlloc.deleteInstance(m_window);
		m_window = nullptr;
	}

	m_settingsDir.destroy(m_heapAlloc);
	m_cacheDir.destroy(m_heapAlloc);

#if ANKI_ENABLE_TRACE
	TraceManagerSingleton::destroy();
#endif
}

Error App::init(const ConfigSet& config, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	Error err = initInternal(config, allocCb, allocCbUserData);
	if(err)
	{
		cleanup();
		ANKI_CORE_LOGE("App initialization failed");
	}

	return err;
}

Error App::initInternal(const ConfigSet& config_, AllocAlignedCallback allocCb, void* allocCbUserData)
{
	m_allocCb = allocCb;
	m_allocCbData = allocCbUserData;
	m_heapAlloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	ConfigSet config = config_;

	ANKI_CHECK(initDirs());

	// Print a message
	const char* buildType =
#if ANKI_OPTIMIZE
		"optimized, "
#else
		"NOT optimized, "
#endif
#if ANKI_DEBUG_SYMBOLS
		"dbg symbols, "
#else
		"NO dbg symbols, "
#endif
#if ANKI_EXTRA_CHECKS
		"extra checks";
#else
		"NO extra checks";
#endif

	ANKI_CORE_LOGI("Initializing application ("
				   "version %u.%u, "
				   "%s, "
				   "date %s, "
				   "commit %s)...",
		ANKI_VERSION_MAJOR,
		ANKI_VERSION_MINOR,
		buildType,
		__DATE__,
		ANKI_REVISION);

	m_timerTick = 1.0 / 60.0; // in sec. 1.0 / period

// Check SIMD support
#if ANKI_SIMD == ANKI_SIMD_SSE
	if(!__builtin_cpu_supports("sse4.2"))
	{
		ANKI_CORE_LOGF("AnKi is built with sse4.2 support but your CPU doesn't "
					   "support it. Try bulding without SSE support");
	}
#endif

#if ANKI_ENABLE_TRACE
	ANKI_CHECK(TraceManagerSingleton::get().create(m_heapAlloc, m_settingsDir.toCString()));
#endif

	//
	// Window
	//
	NativeWindowInitInfo nwinit;
	nwinit.m_width = config.getNumber("width");
	nwinit.m_height = config.getNumber("height");
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = config.getNumber("fullscreenDesktopResolution");
	m_window = m_heapAlloc.newInstance<NativeWindow>();

	ANKI_CHECK(m_window->init(nwinit, m_heapAlloc));

	//
	// Input
	//
	m_input = m_heapAlloc.newInstance<Input>();

	ANKI_CHECK(m_input->create(m_window));

	//
	// ThreadPool
	//
	m_threadpool = m_heapAlloc.newInstance<ThreadPool>(getCpuCoresCount());
	m_threadHive = m_heapAlloc.newInstance<ThreadHive>(getCpuCoresCount(), m_heapAlloc);

	//
	// Graphics API
	//
	m_gr = m_heapAlloc.newInstance<GrManager>();

	GrManagerInitInfo grInit;
	grInit.m_allocCallback = m_allocCb;
	grInit.m_allocCallbackUserData = m_allocCbData;
	grInit.m_cacheDirectory = m_cacheDir.toCString();
	grInit.m_config = &config;
	grInit.m_window = m_window;

	ANKI_CHECK(m_gr->init(grInit));

	//
	// Staging mem
	//
	m_stagingMem = m_heapAlloc.newInstance<StagingGpuMemoryManager>();
	ANKI_CHECK(m_stagingMem->init(m_gr, config));

	//
	// Physics
	//
	m_physics = m_heapAlloc.newInstance<PhysicsWorld>();

	ANKI_CHECK(m_physics->create(m_allocCb, m_allocCbData));

	//
	// Resource FS
	//
	m_resourceFs = m_heapAlloc.newInstance<ResourceFilesystem>(m_heapAlloc);
	ANKI_CHECK(m_resourceFs->init(config, m_cacheDir.toCString()));

	//
	// Resources
	//
	ResourceManagerInitInfo rinit;
	rinit.m_gr = m_gr;
	rinit.m_stagingMem = m_stagingMem;
	rinit.m_physics = m_physics;
	rinit.m_resourceFs = m_resourceFs;
	rinit.m_config = &config;
	rinit.m_cacheDir = m_cacheDir.toCString();
	rinit.m_allocCallback = m_allocCb;
	rinit.m_allocCallbackData = m_allocCbData;
	m_resources = m_heapAlloc.newInstance<ResourceManager>();

	ANKI_CHECK(m_resources->create(rinit));

	//
	// Renderer
	//
	if(nwinit.m_fullscreenDesktopRez)
	{
		config.set("width", m_window->getWidth());
		config.set("height", m_window->getHeight());
	}

	m_renderer = m_heapAlloc.newInstance<MainRenderer>();

	ANKI_CHECK(m_renderer->create(
		m_threadpool, m_resources, m_gr, m_stagingMem, m_allocCb, m_allocCbData, config, &m_globalTimestamp));

	m_resources->_setShadersPrependedSource(m_renderer->getMaterialShaderSource().toCString());

	//
	// Scene
	//
	m_scene = m_heapAlloc.newInstance<SceneGraph>();

	ANKI_CHECK(m_scene->init(
		m_allocCb, m_allocCbData, m_threadpool, m_threadHive, m_resources, m_input, &m_globalTimestamp, config));

	//
	// Script
	//
	m_script = m_heapAlloc.newInstance<ScriptManager>();

	ANKI_CHECK(m_script->init(m_allocCb, m_allocCbData, m_scene, m_renderer));

	ANKI_CORE_LOGI("Application initialized");
	return ErrorCode::NONE;
}

Error App::initDirs()
{
#if ANKI_OS != ANKI_OS_ANDROID
	// Settings path
	StringAuto home(m_heapAlloc);
	ANKI_CHECK(getHomeDirectory(m_heapAlloc, home));

	m_settingsDir.sprintf(m_heapAlloc, "%s/.anki", &home[0]);

	if(!directoryExists(m_settingsDir.toCString()))
	{
		ANKI_CORE_LOGI("Creating settings dir \"%s\"", &m_settingsDir[0]);
		ANKI_CHECK(createDirectory(m_settingsDir.toCString()));
	}
	else
	{
		ANKI_CORE_LOGI("Using settings dir \"%s\"", &m_settingsDir[0]);
	}

	// Cache
	m_cacheDir.sprintf(m_heapAlloc, "%s/cache", &m_settingsDir[0]);

	if(directoryExists(m_cacheDir.toCString()))
	{
		ANKI_CHECK(removeDirectory(m_cacheDir.toCString()));
	}

	ANKI_CHECK(createDirectory(m_cacheDir.toCString()));
#else
	// ANKI_ASSERT(gAndroidApp);
	// ANativeActivity* activity = gAndroidApp->activity;

	// Settings path
	// settingsDir = String(activity->internalDataDir, alloc);
	settingsDir = String("/sdcard/.anki/");
	if(!directoryExists(settingsDir.c_str()))
	{
		createDirectory(settingsDir.c_str());
	}

	// Cache
	cacheDir = settingsDir + "/cache";
	if(directoryExists(cacheDir.c_str()))
	{
		removeDirectory(cacheDir.c_str());
	}

	createDirectory(cacheDir.c_str());
#endif

	return ErrorCode::NONE;
}

Error App::mainLoop()
{
	ANKI_CORE_LOGI("Entering main loop");
	Bool quit = false;

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	while(!quit)
	{
		ANKI_TRACE_START_FRAME();
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		m_gr->beginFrame();

		// Update
		ANKI_CHECK(m_input->handleEvents());

		// User update
		ANKI_CHECK(userMainLoop(quit));

		ANKI_CHECK(m_scene->update(prevUpdateTime, crntTime, *m_renderer));

		ANKI_CHECK(m_renderer->render(*m_scene));

		// Pause and sync async loader. That will force all tasks before the pause to finish in this frame.
		m_resources->getAsyncLoader().pause();

		m_gr->swapBuffers();
		m_stagingMem->endFrame();

		// Update the trace info with some async loader stats
		U64 asyncTaskCount = m_resources->getAsyncLoader().getCompletedTaskCount();
		ANKI_TRACE_INC_COUNTER(RESOURCE_ASYNC_TASKS, asyncTaskCount - m_resourceCompletedAsyncTaskCount);
		m_resourceCompletedAsyncTaskCount = asyncTaskCount;

		// Now resume the loader
		m_resources->getAsyncLoader().resume();

		// Sleep
		timer.stop();
		if(timer.getElapsedTime() < m_timerTick)
		{
			HighRezTimer::sleep(m_timerTick - timer.getElapsedTime());
		}

		++m_globalTimestamp;

		ANKI_TRACE_STOP_FRAME();
	}

	return ErrorCode::NONE;
}

} // end namespace anki

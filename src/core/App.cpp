// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/core/App.h"
#include "anki/misc/ConfigSet.h"
#include "anki/core/Logger.h"
#include "anki/util/Exception.h"
#include "anki/util/File.h"
#include "anki/util/Filesystem.h"
#include "anki/util/System.h"
#include "anki/core/Counters.h"

#include "anki/core/NativeWindow.h"
#include "anki/input/Input.h"
#include "anki/scene/SceneGraph.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/script/ScriptManager.h"

#include <signal.h>
#if ANKI_OS == ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

// Sybsystems

namespace anki {

#if ANKI_OS == ANKI_OS_ANDROID
/// The one and only android hack
android_app* gAndroidApp = nullptr;
#endif

//==============================================================================
App::App(const ConfigSet& config, 
	AllocAlignedCallback allocCb, void* allocCbUserData)
:	m_allocCb(allocCb),
	m_allocCbData(allocCbUserData),
	m_heapAlloc(HeapMemoryPool(allocCb, allocCbUserData))
{
	try
	{
		init(config);
	}
	catch(const std::exception& e)
	{
		cleanup();
		throw ANKI_EXCEPTION("App initialization failed") << e;
	}
}

//==============================================================================
App::~App()
{
	cleanup();
}

//==============================================================================
void App::cleanup()
{
	if (m_script != nullptr)
	{
		m_heapAlloc.deleteInstance(m_script);
		m_script = nullptr;
	}

	if (m_scene != nullptr)
	{
		m_heapAlloc.deleteInstance(m_scene);
		m_scene = nullptr;
	}

	if (m_renderer != nullptr)
	{
		m_heapAlloc.deleteInstance(m_renderer);
		m_renderer = nullptr;
	}

	if (m_resources != nullptr)
	{
		m_heapAlloc.deleteInstance(m_resources);
		m_resources = nullptr;
	}

	if (m_gl != nullptr)
	{
		m_heapAlloc.deleteInstance(m_gl);
		m_gl = nullptr;
	}

	if (m_threadpool != nullptr)
	{
		m_heapAlloc.deleteInstance(m_threadpool);
		m_threadpool = nullptr;
	}

	if (m_input != nullptr)
	{
		m_heapAlloc.deleteInstance(m_input);
		m_input = nullptr;
	}

	if (m_window != nullptr)
	{
		m_heapAlloc.deleteInstance(m_window);
		m_window = nullptr;
	}
}

//==============================================================================
void App::init(const ConfigSet& config)
{
	initDirs();

	// Logger
	LoggerSingleton::init(
		Logger::InitFlags::WITH_SYSTEM_MESSAGE_HANDLER, m_heapAlloc,
		&m_cacheDir[0]);

	// Print a message
	String msg(getAllocator());
	msg.sprintf(
		"Initializing application ("
		"version %u.%u, "
		"build %s %s, "
		"commit %s)...",
		ANKI_VERSION_MAJOR, ANKI_VERSION_MINOR,
#if !ANKI_DEBUG
		"release",
#else
		"debug",
#endif
		__DATE__,
		ANKI_REVISION);

	ANKI_LOGI(&msg[0]);

	m_timerTick = 1.0 / 60.0; // in sec. 1.0 / period

	// Window
	NativeWindow::Initializer nwinit;
	nwinit.m_width = config.get("width");
	nwinit.m_height = config.get("height");
	nwinit.m_majorVersion = config.get("glmajor");
	nwinit.m_minorVersion = config.get("glminor");
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = config.get("fullscreen");
	nwinit.m_debugContext = ANKI_DEBUG;
	m_window = m_heapAlloc.newInstance<NativeWindow>(nwinit, m_heapAlloc);	
	m_ctx = m_window->getCurrentContext();
	m_window->contextMakeCurrent(nullptr);

	// Input
	m_input = m_heapAlloc.newInstance<Input>(m_window);

	// Threadpool
	m_threadpool = m_heapAlloc.newInstance<Threadpool>(getCpuCoresCount());

	// GL
	m_gl = m_heapAlloc.newInstance<GlDevice>(
		m_allocCb, m_allocCbData,
		m_cacheDir.toCString());

	m_gl->startServer(
		makeCurrent, this, m_ctx,
		swapWindow, m_window,
		nwinit.m_debugContext);

	// Resources
	ResourceManager::Initializer rinit;
	rinit.m_gl = m_gl;
	rinit.m_config = &config;
	rinit.m_cacheDir = m_cacheDir.toCString();
	rinit.m_allocCallback = m_allocCb;
	rinit.m_allocCallbackData = m_allocCbData;
	rinit.m_tempAllocatorMemorySize = 1024 * 1024 * 4;
	m_resources = m_heapAlloc.newInstance<ResourceManager>(rinit);

	// Renderer
	m_renderer = m_heapAlloc.newInstance<MainRenderer>(
		m_threadpool,
		m_resources,
		m_gl,
		m_heapAlloc,
		config);

	m_resources->_setShadersPrependedSource(
		m_renderer->_getShadersPrependedSource().toCString());

	// Scene
	m_scene = m_heapAlloc.newInstance<SceneGraph>(
		m_allocCb, m_allocCbData, m_threadpool, m_resources);

	// Script
	m_script = m_heapAlloc.newInstance<ScriptManager>(m_heapAlloc, m_scene);

	ANKI_LOGI("Application initialized");
}

//==============================================================================
void App::makeCurrent(void* papp, void* ctx)
{
	ANKI_ASSERT(papp != nullptr);
	App* app = reinterpret_cast<App*>(papp);
	app->m_window->contextMakeCurrent(ctx);
}

//==============================================================================
void App::swapWindow(void* window)
{
	reinterpret_cast<NativeWindow*>(window)->swapBuffers();
}

//==============================================================================
void App::initDirs()
{
#if ANKI_OS != ANKI_OS_ANDROID
	// Settings path
	String home = getHomeDirectory(m_heapAlloc);
	String settingsDir = home + "/.anki";
	if(!directoryExists(settingsDir.toCString()))
	{
		createDirectory(settingsDir.toCString());
	}

	// Cache
	m_cacheDir = settingsDir + "/cache";
	if(directoryExists(m_cacheDir.toCString()))
	{
		removeDirectory(m_cacheDir.toCString());
	}

	createDirectory(m_cacheDir.toCString());
#else
	//ANKI_ASSERT(gAndroidApp);
	//ANativeActivity* activity = gAndroidApp->activity;

	// Settings path
	//settingsDir = String(activity->internalDataDir, alloc);
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
}

//==============================================================================
void App::mainLoop(UserMainLoopCallback callback, void* userData)
{
	ANKI_LOGI("Entering main loop");
	I32 userRetCode = 0;

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	ANKI_COUNTER_START_TIMER(FPS);
	while(userRetCode == 0)
	{
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		m_input->handleEvents();
		m_scene->update(prevUpdateTime, crntTime, *m_renderer);
		m_renderer->render(*m_scene);

		// User update
		userRetCode = callback(*this, userData);

		m_gl->swapBuffers();
		ANKI_COUNTERS_RESOLVE_FRAME();

		// Sleep
		timer.stop();
		if(timer.getElapsedTime() < getTimerTick())
		{
			HighRezTimer::sleep(getTimerTick() - timer.getElapsedTime());
		}

		// Timestamp
		increaseGlobTimestamp();
	}

	// Counters end
	ANKI_COUNTER_STOP_TIMER_INC(FPS);
	ANKI_COUNTERS_FLUSH();
}

} // end namespace anki

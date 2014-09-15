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
	init(config);
}

//==============================================================================
void App::makeCurrent(void* data)
{
	App* app = reinterpret_cast<App*>(data);
	app->m_window->contextMakeCurrent(app->m_ctx);
}

//==============================================================================
void App::swapWindow(void* window)
{
	reinterpret_cast<NativeWindow*>(window)->swapBuffers();
}

//==============================================================================
void App::init(const ConfigSet& config)
{
	// Logger
	LoggerSingleton::init(
		Logger::InitFlags::WITH_SYSTEM_MESSAGE_HANDLER, m_heapAlloc,
		&m_cacheDir[0]);

	printAppInfo();
	initDirs();

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
		makeCurrent, this,
		swapWindow, m_window,
		nwinit.m_debugContext);

	// Resources
	ResourceManager::Initializer rinit;
	rinit.m_gl = m_gl;
	rinit.m_config = &config;
	rinit.m_cacheDir = m_cacheDir.toCString();
	rinit.m_allocCallback = m_allocCb;
	rinit.m_allocCallbackData = m_allocCbData;
	m_resources = m_heapAlloc.newInstance<ResourceManager>(rinit);

	// Renderer
	m_renderer = m_heapAlloc.newInstance<MainRenderer>(
		m_threadpool,
		m_resources,
		m_gl,
		m_heapAlloc,
		config);

	// Scene
	m_scene = m_heapAlloc.newInstance<SceneGraph>(
		m_allocCb, m_allocCbData, m_threadpool, m_resources);

	// Script
	m_script = m_heapAlloc.newInstance<ScriptManager>(m_heapAlloc);
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
		ANKI_LOGI("Creating settings dir: %s", &settingsDir[0]);
		createDirectory(settingsDir.toCString());
	}

	// Cache
	m_cacheDir = settingsDir + "/cache";
	if(directoryExists(m_cacheDir.toCString()))
	{
		ANKI_LOGI("Deleting dir: %s", &m_cacheDir[0]);
		removeDirectory(m_cacheDir.toCString());
	}

	ANKI_LOGI("Creating cache dir: %s", &m_cacheDir[0]);
	createDirectory(m_cacheDir.toCString());
#else
	//ANKI_ASSERT(gAndroidApp);
	//ANativeActivity* activity = gAndroidApp->activity;

	// Settings path
	//settingsDir = String(activity->internalDataDir, alloc);
	settingsDir = String("/sdcard/.anki/");
	if(!directoryExists(settingsDir.c_str()))
	{
		ANKI_LOGI("Creating settings dir: %s", settingsDir.c_str());
		createDirectory(settingsDir.c_str());
	}

	// Cache
	cacheDir = settingsDir + "/cache";
	if(directoryExists(cacheDir.c_str()))
	{
		ANKI_LOGI("Deleting dir: %s", cacheDir.c_str());
		removeDirectory(cacheDir.c_str());
	}

	ANKI_LOGI("Creating cache dir: %s", cacheDir.c_str());
	createDirectory(cacheDir.c_str());
#endif
}

//==============================================================================
void App::quit(int code)
{}

//==============================================================================
void App::printAppInfo()
{
	String msg(getAllocator());
	msg.sprintf(
		"App info: "
		"version %u.%u, "
		"build %s %s, "
		"commit %s",
		ANKI_VERSION_MAJOR, ANKI_VERSION_MINOR,
#if !ANKI_DEBUG
		"Release",
#else
		"Debug",
#endif
		__DATE__,
		ANKI_REVISION);

	ANKI_LOGI(&msg[0]);
}

//==============================================================================
void App::mainLoop(UserMainLoopCallback callback, void* userData)
{
	ANKI_LOGI("Entering main loop");

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	ANKI_COUNTER_START_TIMER(FPS);
	while(true)
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
		callback(*this, userData);

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

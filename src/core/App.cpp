// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/core/App.h"
#include "anki/Config.h"
#include "anki/core/Logger.h"
#include "anki/util/Exception.h"
#include "anki/util/File.h"
#include "anki/util/System.h"
#include "anki/scene/SceneGraph.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/input/Input.h"
#include "anki/core/NativeWindow.h"
#include "anki/core/Counters.h"
#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
//#include <execinfo.h> XXX
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
App::App(AllocAlignedCallback allocCb, void* allocCbUserData)
:	m_allocCb(allocCb),
	m_allocCbData(allocCbUserData),
	m_heapAlloc(HeapMemoryPool(allocCbUserData, allocCb))
{
	init(config);
}

//==============================================================================
void App::init(const Config& config)
{
	printAppInfo();
	initDirs();

	timerTick = 1.0 / 60.0; // in sec. 1.0 / period

	// Logger
	LoggerSingleton::init(
		Logger::InitFlags::WITH_SYSTEM_MESSAGE_HANDLER, m_heapAlloc);

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
	Context context = m_window->getCurrentContext();
	m_window->contextMakeCurrent(nullptr);

	// Input
	m_input = m_heapAlloc.newInstance<Input>(m_window);

	// Threadpool
	m_threadpool = m_heapAlloc.newInstance<Threadpool>(getCpuCoresCount());

	// Scene
	m_scene = m_heapAlloc.newInstance<SceneGraph>(
		m_allocCb, m_allocCbData, m_threadpool);
}

//==============================================================================
void App::initDirs()
{
#if ANKI_OS != ANKI_OS_ANDROID
	// Settings path
	Array<char, 512> home;
	getHomeDirectory(sizeof(home), &home[0]);
	String settingsPath = String(&home[0]) + "/.anki";
	if(!directoryExists(settingsPath.c_str()))
	{
		ANKI_LOGI("Creating settings dir: %s", settingsPath.c_str());
		createDirectory(settingsPath.c_str());
	}

	// Cache
	cachePath = settingsPath + "/cache";
	if(directoryExists(cachePath.c_str()))
	{
		ANKI_LOGI("Deleting dir: %s", cachePath.c_str());
		removeDirectory(cachePath.c_str());
	}

	ANKI_LOGI("Creating cache dir: %s", cachePath.c_str());
	createDirectory(cachePath.c_str());
#else
	//ANKI_ASSERT(gAndroidApp);
	//ANativeActivity* activity = gAndroidApp->activity;

	// Settings path
	//settingsPath = String(activity->internalDataPath, alloc);
	settingsPath = String("/sdcard/.anki/");
	if(!directoryExists(settingsPath.c_str()))
	{
		ANKI_LOGI("Creating settings dir: %s", settingsPath.c_str());
		createDirectory(settingsPath.c_str());
	}

	// Cache
	cachePath = settingsPath + "/cache";
	if(directoryExists(cachePath.c_str()))
	{
		ANKI_LOGI("Deleting dir: %s", cachePath.c_str());
		removeDirectory(cachePath.c_str());
	}

	ANKI_LOGI("Creating cache dir: %s", cachePath.c_str());
	createDirectory(cachePath.c_str());
#endif
}

//==============================================================================
void App::quit(int code)
{}

//==============================================================================
void App::printAppInfo()
{
	std::stringstream msg;
	msg << "App info: ";
	msg << "AnKi " << ANKI_VERSION_MAJOR << "." << ANKI_VERSION_MINOR << ", ";
#if !ANKI_DEBUG
	msg << "Release";
#else
	msg << "Debug";
#endif
	msg << " build,";

	msg << " " << ANKI_CPU_ARCH_STR;
	msg << " " << ANKI_GL_STR;
	msg << " " << ANKI_WINDOW_BACKEND_STR;
	msg << " CPU cores " << getCpuCoresCount();

	msg << " build date " __DATE__ ", " << "rev " << ANKI_REVISION;

	ANKI_LOGI(msg.str().c_str());
}

//==============================================================================
void App::mainLoop()
{
#if 0
	ANKI_LOGI("Entering main loop");

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	SceneGraph& scene = SceneGraphSingleton::get();
	MainRenderer& renderer = MainRendererSingleton::get();
	Input& input = InputSingleton::get();
	NativeWindow& window = NativeWindowSingleton::get();

	ANKI_COUNTER_START_TIMER(FPS);
	while(true)
	{
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		input.handleEvents();
		scene.update(
			prevUpdateTime, crntTime, MainRendererSingleton::get());
		renderer.render(SceneGraphSingleton::get());

		window.swapBuffers();
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
#endif
}

} // end namespace anki

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "anki/input/Input.h"
#include "anki/Math.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/SkelAnim.h"
#include "anki/physics/Character.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/physics/Character.h"
#include "anki/physics/RigidBody.h"
#include "anki/script/ScriptManager.h"
#include "anki/core/StdinListener.h"
#include "anki/resource/Model.h"
#include "anki/core/Logger.h"
#include "anki/Util.h"
#include "anki/resource/Skin.h"
#include "anki/event/EventManager.h"
#include "anki/event/MainRendererPpsHdrEvent.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/Material.h"
#include "anki/core/ThreadPool.h"
#include "anki/core/NativeWindow.h"
#include "anki/core/Counters.h"
#include "anki/Scene.h"
#include <android_native_app_glue.h>
#include <android/log.h>

using namespace anki;

//==============================================================================
void initSubsystems(android_app* app)
{
	U glmajor = 3, glminor = 0;

	// App
	AppSingleton::get().init(app);

	// Util
	File::setAndroidAssetManager(app->activity->assetManager);

	// Window
	NativeWindowInitializer nwinit;
	nwinit.width = 1280;
	nwinit.height = 720;
	nwinit.majorVersion = glmajor;
	nwinit.minorVersion = glminor;
	nwinit.depthBits = 0;
	nwinit.stencilBits = 0;
	nwinit.fullscreenDesktopRez = true;
	nwinit.debugContext = false;
	NativeWindowSingleton::get().create(nwinit);

	// GL stuff
	GlStateCommonSingleton::get().init(glmajor, glminor, nwinit.debugContext);

	// Input
	InputSingleton::get().init(&NativeWindowSingleton::get());
	InputSingleton::get().lockCursor(true);
	InputSingleton::get().hideCursor(true);
	InputSingleton::get().moveCursor(Vec2(0.0));

	// Main renderer
	RendererInitializer initializer;
	initializer.ms.ez.enabled = false;
	initializer.ms.ez.maxObjectsToDraw = 100;
	initializer.dbg.enabled = false;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.groundLightEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = false;
	initializer.is.sm.resolution = 512;
	initializer.pps.enabled = true;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.5;
	initializer.pps.hdr.blurringDist = 1.0;
	initializer.pps.hdr.blurringIterationsCount = 1;
	initializer.pps.hdr.exposure = 8.0;
	initializer.pps.ssao.blurringIterationsNum = 1;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.mainPassRenderingQuality = 0.35;
	initializer.pps.ssao.blurringRenderingQuality = 0.35;
	initializer.pps.bl.enabled = true;
	initializer.pps.bl.blurringIterationsNum = 2;
	initializer.pps.bl.sideBlurFactor = 1.0;
	initializer.pps.lf.enabled = true;
	initializer.pps.sharpen = true;
	initializer.renderingQuality = 1.0;
	initializer.width = NativeWindowSingleton::get().getWidth();
	initializer.height = NativeWindowSingleton::get().getHeight();
	initializer.lodDistance = 20.0;
	initializer.samples = 16;

#if ANKI_GL == ANKI_GL_ES
	initializer.samples = 1;
	initializer.pps.enabled = false;
	initializer.is.maxPointLights = 64;
	initializer.is.maxPointLightsPerTile = 4;
	initializer.is.maxSpotLightsPerTile = 4;
	initializer.is.maxSpotTexLightsPerTile = 4;
#endif

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ThreadPoolSingleton::get().init(getCpuCoresCount());
}

//==============================================================================
static void handleEvents(android_app* app, int32_t cmd) 
{
	switch(cmd) 
	{
	case APP_CMD_SAVE_STATE:
		ANKI_LOGI("APP_CMD_SAVE_STATE");
		break;
	case APP_CMD_INIT_WINDOW:
		ANKI_LOGI("APP_CMD_INIT_WINDOW");
		break;
	case APP_CMD_TERM_WINDOW:
		ANKI_LOGI("APP_CMD_TERM_WINDOW");
		break;
	case APP_CMD_GAINED_FOCUS:
		ANKI_LOGI("APP_CMD_GAINED_FOCUS");
		break;
	case APP_CMD_LOST_FOCUS:
		ANKI_LOGI("APP_CMD_LOST_FOCUS");
		break;
	}
}

//==============================================================================
void mainLoop()
{
	ANKI_LOGI("Entering main loop");

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	SceneGraph& scene = SceneGraphSingleton::get();
	MainRenderer& renderer = MainRendererSingleton::get();
	Input& input = InputSingleton::get();
	NativeWindow& window = NativeWindowSingleton::get();

	ANKI_COUNTER_START_TIMER(C_FPS);
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
		if(timer.getElapsedTime() < AppSingleton::get().getTimerTick())
		{
			HighRezTimer::sleep(
				AppSingleton::get().getTimerTick() - timer.getElapsedTime());
		}

		// Timestamp
		increaseGlobTimestamp();
	}

	// Counters end
	ANKI_COUNTER_STOP_TIMER_INC(C_FPS);
	ANKI_COUNTERS_FLUSH();
}

//==============================================================================
void loopUntilWindowIsReady(android_app* app)
{
	while(app->window == nullptr) 
	{
		int ident;
		int events;
		android_poll_source* source;

		while((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) 
		{
			if (source != NULL) 
			{
				source->process(app, source);
			}
		}
	}
}

//==============================================================================
void android_main(android_app* app)
{
	app_dummy();

	app->onAppCmd = handleEvents;
	loopUntilWindowIsReady(app);

	try
	{
		initSubsystems(app);
		mainLoop();

		ANKI_LOGI("Exiting...");
	}
	catch(std::exception& e)
	{
		ANKI_LOGE("Aborting: %s", e.what());
	}

	ANKI_LOGI("Bye!!");
	exit(1);
}

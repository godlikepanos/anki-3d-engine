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
#include "anki/event/LightEvent.h"
#include "anki/event/MovableEvent.h"

using namespace anki;

//==============================================================================
struct LogFile 
{
	ANKI_HAS_SLOTS(LogFile)

	void handler(const Logger::Info& info)
	{
		const char* x;
		switch(info.type)
		{
		case Logger::LMT_NORMAL:
			x = "Info";
			break;
		case Logger::LMT_ERROR:
			x = "Error";
			break;
		case Logger::LMT_WARNING:
			x = "Warn";
			break;
		}

		file.writeText("(%s:%d %s) %s: %s\n", 
			info.file, info.line, info.func, x, info.msg);
	}
	ANKI_SLOT(handler, const Logger::Info&)

	File file;
};

static LogFile logfile;

//==============================================================================
void initSubsystems()
{
#if ANKI_OS == ANKI_OS_ANDROID
	// Log file
	logfile.file.open("/sdcard/anki.log", File::OF_WRITE);
	ANKI_CONNECT(&LoggerSingleton::get(), messageRecieved, &logfile, handler);
#endif

	// App
	AppSingleton::get().init();

	// Window
	NativeWindowInitializer nwinit;
	nwinit.width = 1280;
	nwinit.height = 720;
#if ANKI_GL == ANKI_GL_ES
	nwinit.majorVersion = 3;
	nwinit.minorVersion = 0;
#else
	nwinit.majorVersion = 4;
	nwinit.minorVersion = 3;
#endif
	nwinit.depthBits = 0;
	nwinit.stencilBits = 0;
	nwinit.fullscreenDesktopRez = true;
	nwinit.debugContext = false;
	NativeWindowSingleton::get().create(nwinit);

	// GL stuff
	GlStateCommonSingleton::get().init(
		nwinit.majorVersion, nwinit.minorVersion, nwinit.debugContext);

	// Input
	InputSingleton::get().init(&NativeWindowSingleton::get());
	InputSingleton::get().lockCursor(true);
	InputSingleton::get().hideCursor(true);
	InputSingleton::get().moveCursor(Vec2(0.0));

	// Main renderer
	RendererInitializer initializer;
	initializer.get("ms.ez.enabled") = false;
	initializer.get("ms.ez.maxObjectsToDraw") = 100;
	initializer.get("dbg.enabled") = false;
	initializer.get("is.sm.bilinearEnabled") = true;
	initializer.get("is.groundLightEnabled") = true;
	initializer.get("is.sm.enabled") = true;
	initializer.get("is.sm.pcfEnabled") = false;
	initializer.get("is.sm.resolution") = 512;
	initializer.get("pps.enabled") = true;
	initializer.get("pps.hdr.enabled") = true;
	initializer.get("pps.hdr.renderingQuality") = 0.5;
	initializer.get("pps.hdr.blurringDist") = 1.0;
	initializer.get("pps.hdr.blurringIterationsCount") = 1;
	initializer.get("pps.hdr.exposure") = 8.0;
	initializer.get("pps.ssao.blurringIterationsNum") = 1;
	initializer.get("pps.ssao.enabled") = true;
	initializer.get("pps.ssao.mainPassRenderingQuality") = 0.35;
	initializer.get("pps.ssao.blurringRenderingQuality") = 0.35;
	initializer.get("pps.bl.enabled") = true;
	initializer.get("pps.bl.blurringIterationsNum") = 2;
	initializer.get("pps.bl.sideBlurFactor") = 1.0;
	initializer.get("pps.lf.enabled") = true;
	initializer.get("pps.sharpen") = true;
	initializer.get("renderingQuality") = 1.0;
	initializer.get("width") = NativeWindowSingleton::get().getWidth();
	initializer.get("height") = NativeWindowSingleton::get().getHeight();
	initializer.get("lodDistance") = 20.0;
	initializer.get("samples") = 16;

#if 1
	initializer.get("samples") = 1;
	initializer.get("is.maxPointLights") = 64;
	initializer.get("is.maxPointLightsPerTile") = 4;
	initializer.get("is.maxSpotLightsPerTile") = 4;
	initializer.get("is.maxSpotTexLightsPerTile") = 4;
	initializer.get("pps.enabled") = false;
	initializer.get("renderingQuality") = 0.8;
	initializer.get("maxTextureSize") = 256;
#endif

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ThreadPoolSingleton::get().init(getCpuCoresCount());
}

//==============================================================================
void initScene()
{
	SceneGraph& scene = SceneGraphSingleton::get();

	scene.setAmbientColor(Vec4(0.1, 0.05, 0.05, 0.0) * 2);

	PerspectiveCamera* cam = new PerspectiveCamera(
		"main_camera", &scene, nullptr, Movable::MF_NONE);

	const F32 ang = 45.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 500.0);
	cam->setLocalTransform(Transform(Vec3(18.0, 5.2, 0.0),
		Mat3(Euler(toRad(-10.0), toRad(90.0), toRad(0.0))),
		1.0));
	scene.setActiveCamera(cam);

#if 1
	F32 x = 8.5; 
	F32 y = 2.25;
	F32 z = 2.49;
	Array<Vec3, 4> vaseLightPos = {{Vec3(x, y, -z - 1.4), Vec3(x, y, z),
		Vec3(-x - 2.3, y, z), Vec3(-x - 2.3, y, -z - 1.4)}};
	for(U i = 0; i < vaseLightPos.getSize(); i++)
	{
		Vec3 lightPos = vaseLightPos[i];

		PointLight* point =
			new PointLight(("vase_plight" + std::to_string(i)).c_str(),
			&scene, nullptr, Movable::MF_NONE, 
			(i != 100) ? "textures/lens_flare/flares0.ankitex" : nullptr);
		point->setRadius(2.0);
		point->setLocalOrigin(lightPos);
		point->setDiffuseColor(Vec4(3.0, 0.2, 0.0, 0.0));
		point->setSpecularColor(Vec4(1.0, 1.0, 0.0, 0.0));
		point->setLensFlaresStretchMultiplier(Vec2(10.0, 1.0));
		point->setLensFlaresAlpha(1.0);

		LightEventData eventData;
		eventData.light = point;
		eventData.radiusMultiplier = 0.2;
		eventData.intensityMultiplier = Vec4(-1.2, 0.0, 0.0, 0.0);
		eventData.specularIntensityMultiplier = Vec4(0.1, 0.1, 0.0, 0.0);
		auto event = scene.getEventManager().newLightEvent(0.0, 0.8, eventData);
		event->enableBits(Event::EF_REANIMATE);

		MovableEventData moveData;
		moveData.movableSceneNode = point;
		moveData.posMin = Vec3(-0.5, 0.0, -0.5);
		moveData.posMax = Vec3(0.5, 0.0, 0.5);
		auto mevent = scene.getEventManager().newMovableEvent(0.0, 2.0, moveData);
		mevent->enableBits(Event::EF_REANIMATE);

		ParticleEmitter* pe = new ParticleEmitter(
			("pe" + std::to_string(i)).c_str(), &scene, nullptr,
			Movable::MF_NONE, "particles/smoke.ankipart");
		pe->setLocalOrigin(lightPos);

		pe = new ParticleEmitter(
			("pef" + std::to_string(i)).c_str(), &scene, nullptr,
			Movable::MF_NONE, "particles/fire.ankipart");
		pe->setLocalOrigin(lightPos);
	}
#endif

	scene.load("maps/sponza/master.scene");

	PointLight* pl = new PointLight("pl0", &scene, nullptr, Movable::MF_NONE);
	pl->setRadius(12.5);
	pl->setDiffuseColor(Vec4(0.5, 0.3, 0.2, 1.0));
	pl->setSpecularColor(Vec4(0.1, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10, 2.0, -0.8));

	pl = new PointLight("pl1", &scene, nullptr, Movable::MF_NONE);
	pl->setRadius(12.5);
	pl->setDiffuseColor(Vec4(0.5, 0.3, 0.2, 1.0));
	pl->setSpecularColor(Vec4(0.1, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(0, 2.0, -0.8));

	pl = new PointLight("pl2", &scene, nullptr, Movable::MF_NONE);
	pl->setRadius(12.5);
	pl->setDiffuseColor(Vec4(0.5, 0.3, 0.2, 1.0));
	pl->setSpecularColor(Vec4(0.1, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(-11, 2.0, -0.8));
}

//==============================================================================
#if ANKI_OS == ANKI_OS_ANDROID
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
#endif

//==============================================================================
static Bool mainLoopExtra()
{
	const F32 dist = 0.2;
	const F32 ang = toRad(3.0);
	const F32 scale = 0.01;
	const F32 mouseSensivity = 9.0;

	Input& in = InputSingleton::get();

	Movable* mover = SceneGraphSingleton::get().getActiveCamera().getMovable();

	if(in.getKey(KC_UP)) mover->rotateLocalX(ang);
	if(in.getKey(KC_DOWN)) mover->rotateLocalX(-ang);
	if(in.getKey(KC_LEFT)) mover->rotateLocalY(ang);
	if(in.getKey(KC_RIGHT)) mover->rotateLocalY(-ang);

	if(in.getKey(KC_A)) mover->moveLocalX(-dist);
	if(in.getKey(KC_D)) mover->moveLocalX(dist);
	if(in.getKey(KC_Z)) mover->moveLocalY(dist);
	if(in.getKey(KC_SPACE)) mover->moveLocalY(-dist);
	if(in.getKey(KC_W)) mover->moveLocalZ(-dist);
	if(in.getKey(KC_S)) mover->moveLocalZ(dist);
	if(in.getKey(KC_Q)) mover->rotateLocalZ(ang);
	if(in.getKey(KC_E)) mover->rotateLocalZ(-ang);

	if(in.getMousePosition() != Vec2(0.0))
	{
		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity *
			MainRendererSingleton::get().getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}

	if(InputSingleton::get().getKey(KC_ESCAPE))
	{
		return false;
	}

	return true;
}

//==============================================================================
static void mainLoop()
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
		if(!mainLoopExtra())
		{
			break;
		}
		scene.update(prevUpdateTime, crntTime, renderer);
		renderer.render(scene);

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
#if ANKI_OS == ANKI_OS_ANDROID
void android_main(android_app* app)
{
	app_dummy();

	// First thing to do
	gAndroidApp = app;

	app->onAppCmd = handleEvents;
#else
int main(int, char**)
{
#endif

	try
	{
		initSubsystems();
		initScene();
		mainLoop();

		ANKI_LOGI("Exiting...");
	}
	catch(std::exception& e)
	{
		ANKI_LOGE("Aborting: %s", e.what());
	}

	ANKI_LOGI("Bye!!");
#if ANKI_OS == ANKI_OS_ANDROID
	exit(0);
#else
	return 0;
#endif
}

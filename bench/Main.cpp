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
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/Material.h"
#include "anki/core/Threadpool.h"
#include "anki/core/NativeWindow.h"
#include "anki/core/Counters.h"
#include "anki/Scene.h"
#include "anki/Event.h"

using namespace anki;

//==============================================================================
void initSubsystems()
{
	// Logger
	LoggerSingleton::get().init(
		Logger::INIT_SYSTEM_MESSAGE_HANDLER 
		| Logger::INIT_LOG_FILE_MESSAGE_HANDLER);

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
	nwinit.majorVersion = 3;
	nwinit.minorVersion = 3;
#endif
	nwinit.depthBits = 0;
	nwinit.stencilBits = 0;
	nwinit.fullscreenDesktopRez = false;
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
	initializer.get("is.sm.resolution") = 512;
	initializer.get("pps.enabled") = true;
	initializer.get("pps.hdr.enabled") = true;
	initializer.get("pps.hdr.renderingQuality") = 0.5;
	initializer.get("pps.hdr.blurringDist") = 1.0;
	initializer.get("pps.hdr.blurringIterationsCount") = 1;
	initializer.get("pps.hdr.exposure") = 8.0;
	initializer.get("pps.ssao.enabled") = false;
	initializer.get("pps.ssao.blurringIterationsNum") = 1;
	initializer.get("pps.ssao.renderingQuality") = 0.5;
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

//#if ANKI_GL == ANKI_GL_ES
#if 1
	initializer.get("samples") = 1;
	initializer.get("is.groundLightEnabled") = true;
	initializer.get("is.maxPointLights") = 64;
	initializer.get("is.maxPointLightsPerTile") = 8;
	initializer.get("is.maxSpotLightsPerTile") = 4;
	initializer.get("is.maxSpotTexLightsPerTile") = 4;
	initializer.get("is.sm.poissonEnabled") = false;
	initializer.get("is.sm.bilinearEnabled") = false;
	initializer.get("renderingQuality") = 0.5;
	initializer.get("pps.hdr.enabled") = true;
	initializer.get("pps.hdr.renderingQuality") = 0.4;
	initializer.get("pps.hdr.blurringDist") = 1.0;
	initializer.get("pps.hdr.samples") = 5;
	initializer.get("pps.hdr.blurringIterationsCount") = 1;
	initializer.get("pps.hdr.exposure") = 8.0;
	initializer.get("maxTextureSize") = 1024;
	initializer.get("mrt") = false;
	initializer.get("tilesXCount") = 16;
	initializer.get("tilesYCount") = 16;
	initializer.get("pps.sharpen") = false;
	initializer.get("pps.gammaCorrection") = true;
#endif

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ThreadpoolSingleton::get().init(getCpuCoresCount());
}

//==============================================================================
void initScene()
{
	SceneGraph& scene = SceneGraphSingleton::get();

	scene.setAmbientColor(Vec4(0.1, 0.05, 0.05, 0.0) * 2);

	PerspectiveCamera* cam = nullptr;
	scene.newSceneNode(
		cam, "main_camera");

	const F32 ang = 45.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 500.0);
	cam->setLocalTransform(Transform(Vec3(18.0, 5.2, 0.0),
		Mat3(Euler(toRad(-10.0), toRad(90.0), toRad(0.0))),
		1.0));
	scene.setActiveCamera(cam);

#if 1
	AnimationResourcePointer anim;
	anim.load("maps/sponza/animation_0.ankianim");
	AnimationEvent* event;
	scene.getEventManager().newEvent(event, anim, cam);
#endif

#if 1
	F32 x = 8.5; 
	F32 y = 2.25;
	F32 z = 2.49;
	Array<Vec3, 4> vaseLightPos = {{Vec3(x, y, -z - 1.4), Vec3(x, y, z),
		Vec3(-x - 2.3, y, z), Vec3(-x - 2.3, y, -z - 1.4)}};
	for(U i = 0; i < vaseLightPos.getSize(); i++)
	{
		Vec3 lightPos = vaseLightPos[i];

		PointLight* point;
		scene.newSceneNode(point, ("vase_plight" + std::to_string(i)).c_str());

		point->loadLensFlare("textures/lens_flare/flares0.ankitex");
		point->setRadius(2.0);
		point->setLocalOrigin(lightPos);
		point->setDiffuseColor(Vec4(3.0, 0.2, 0.0, 0.0));
		point->setSpecularColor(Vec4(1.0, 1.0, 0.0, 0.0));
		point->setLensFlaresStretchMultiplier(Vec2(10.0, 1.0));
		point->setLensFlaresAlpha(1.0);

		LightEventData eventData;
		eventData.radiusMultiplier = 0.2;
		eventData.intensityMultiplier = Vec4(-1.2, 0.0, 0.0, 0.0);
		eventData.specularIntensityMultiplier = Vec4(0.1, 0.1, 0.0, 0.0);
		LightEvent* event;
		scene.getEventManager().newEvent(event, 0.0, 0.8, point, eventData);
		event->enableBits(Event::EF_REANIMATE);

		MoveEventData moveData;
		moveData.posMin = Vec3(-0.5, 0.0, -0.5);
		moveData.posMax = Vec3(0.5, 0.0, 0.5);
		MoveEvent* mevent;
		scene.getEventManager().newEvent(mevent, 0.0, 2.0, point, moveData);
		mevent->enableBits(Event::EF_REANIMATE);

		if(i == 0)
		{
			ParticleEmitter* pe;
			scene.newSceneNode(pe, "pefire", "particles/fire.ankipart");
			pe->setLocalOrigin(lightPos);

			scene.newSceneNode(pe, "pesmoke", "particles/smoke.ankipart");
			pe->setLocalOrigin(lightPos);
		}
		else
		{
			InstanceNode* instance;
			scene.newSceneNode(instance, 
				("pefire_inst" + std::to_string(i)).c_str());

			instance->setLocalOrigin(lightPos);

			SceneNode& sn = scene.findSceneNode("pefire");
			sn.addChild(instance);


			scene.newSceneNode(instance, 
				("pesmoke_inst" + std::to_string(i)).c_str());

			instance->setLocalOrigin(lightPos);

			scene.findSceneNode("pesmoke").addChild(instance);
		}

		{
			ParticleEmitter* pe;
			scene.newSceneNode(pe, ("pesparks" + std::to_string(i)).c_str(), 
				"particles/sparks.ankipart");
			pe->setLocalOrigin(lightPos);
		}
	}
#endif

	// horse
	ModelNode* horse;
	scene.newSceneNode(horse, "horse", "models/horse/horse.ankimdl");
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(),
		0.7));

	// Light
	SpotLight* spot;
	scene.newSceneNode(spot, "spot0");
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec3(7.769279, 8.472027, -0.324927),
		Mat3(Quat(-0.267508, 0.664765, 0.244565, 0.653239)), 1.0));
	spot->setDiffuseColor(Vec4(2.0));
	spot->setSpecularColor(Vec4(1.0, 0.0, 1.0, 1.0));
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);

	// Scene
	scene.load("maps/sponza/master.ankiscene");

	PointLight* pl;
	scene.newSceneNode(pl, "pl0");
	pl->setRadius(8.5);
	pl->setDiffuseColor(Vec4(0.5, 0.3, 0.2, 1.0));
	pl->setSpecularColor(Vec4(0.1, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10, 2.0, -0.8));

	/*scene.newSceneNode(pl, "pl1");
	pl->setRadius(12.5);
	pl->setDiffuseColor(Vec4(0.5, 0.3, 0.2, 1.0));
	pl->setSpecularColor(Vec4(0.1, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(0, 2.0, -0.8));*/

	scene.newSceneNode(pl, "pl2");
	pl->setRadius(8.5);
	pl->setDiffuseColor(Vec4(0.5, 0.3, 0.2, 1.0));
	pl->setSpecularColor(Vec4(0.1, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(-11, 2.0, -0.8));


	// bounce lights
	const F32 bounceRadius = 2.0;
	const F32 lightPower = 0.3;
	const F32 posy = 9.844058;
	F32 posz = -5.550180;
	scene.newSceneNode(pl, "bpl0");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, lightPower, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232, posy, posz));

	scene.newSceneNode(pl, "bpl1");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, lightPower, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232, posy, posz));

	scene.newSceneNode(pl, "bpl2");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, 0.0, lightPower, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232 - 6.5, posy, posz));

	scene.newSceneNode(pl, "bpl3");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, 0.0, lightPower, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232 - 6.5, posy, posz));

	scene.newSceneNode(pl, "bpl4");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(lightPower, 0.0, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232 - 13, posy, posz));

	scene.newSceneNode(pl, "bpl5");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(lightPower, 0.0, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232 - 13, posy, posz));

	scene.newSceneNode(pl, "bpl6");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(lightPower, 0.0, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232 - 13 - 6.5, posy, posz));

	scene.newSceneNode(pl, "bpl7");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(lightPower, 0.0, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232 - 13 - 6.5, posy, posz));


	posz = 4.050180;


	scene.newSceneNode(pl, "bpl0_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, 0.0, lightPower, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232, posy, posz));

	/*scene.newSceneNode(pl, "bpl1_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, 0.0, lightPower, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232, posy, posz));*/

	scene.newSceneNode(pl, "bpl2_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(lightPower, 0.0, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232 - 6.5, posy, posz));

	scene.newSceneNode(pl, "bpl3_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(lightPower, 0.0, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232 - 6.5, posy, posz));

	scene.newSceneNode(pl, "bpl4_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, lightPower, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232 - 13, posy, posz));

	scene.newSceneNode(pl, "bpl5_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, lightPower, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232 - 13, posy, posz));

	scene.newSceneNode(pl, "bpl6_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, 0.0, lightPower, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(10.270232 - 13 - 6.5, posy, posz));

	/*scene.newSceneNode(pl, "bpl7_");
	pl->setRadius(bounceRadius);
	pl->setDiffuseColor(Vec4(0.0, lightPower, 0.0, 1.0));
	pl->setSpecularColor(Vec4(0.0, 0.0, 0.0, 1.0));
	pl->setLocalOrigin(Vec3(6.570232 - 13 - 6.5, posy, posz));*/
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
	const F32 mouseSensivity = 9.0;

	Input& in = InputSingleton::get();

	MoveComponent& mover = 
		SceneGraphSingleton::get().getActiveCamera().
		getComponent<MoveComponent>();

	if(in.getKey(KC_UP)) mover.rotateLocalX(ang);
	if(in.getKey(KC_DOWN)) mover.rotateLocalX(-ang);
	if(in.getKey(KC_LEFT)) mover.rotateLocalY(ang);
	if(in.getKey(KC_RIGHT)) mover.rotateLocalY(-ang);

	if(in.getKey(KC_A)) mover.moveLocalX(-dist);
	if(in.getKey(KC_D)) mover.moveLocalX(dist);
	if(in.getKey(KC_Z)) mover.moveLocalY(dist);
	if(in.getKey(KC_SPACE)) mover.moveLocalY(-dist);
	if(in.getKey(KC_W)) mover.moveLocalZ(-dist);
	if(in.getKey(KC_S)) mover.moveLocalZ(dist);
	if(in.getKey(KC_Q)) mover.rotateLocalZ(ang);
	if(in.getKey(KC_E)) mover.rotateLocalZ(-ang);

	if(in.getMousePosition() != Vec2(0.0))
	{
		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity *
			MainRendererSingleton::get().getAspectRatio();

		mover.rotateLocalY(angY);
		mover.rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}

	if(in.getKey(KC_F1) == 1)
	{
		MainRendererSingleton::get().getDbg().setEnabled(
			!MainRendererSingleton::get().getDbg().getEnabled());
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

		if(input.getEvent(Input::WINDOW_CLOSED_EVENT) > 0)
		{
			break;
		}

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

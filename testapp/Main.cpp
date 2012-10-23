#include <stdio.h>
#include <iostream>
#include <fstream>

#include "anki/input/Input.h"
#include "anki/scene/Camera.h"
#include "anki/math/Math.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/resource/Mesh.h"
#include "anki/scene/Light.h"
#include "anki/resource/Material.h"
#include "anki/scene/Scene.h"
#include "anki/resource/SkelAnim.h"
#include "anki/scene/ParticleEmitterNode.h"
#include "anki/physics/Character.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/physics/Character.h"
#include "anki/physics/RigidBody.h"
#include "anki/script/ScriptManager.h"
#include "anki/core/StdinListener.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/core/Logger.h"
#include "anki/util/Filesystem.h"
#include "anki/util/HighRezTimer.h"
#include "anki/scene/SkinNode.h"
#include "anki/resource/Skin.h"
#include "anki/event/EventManager.h"
#include "anki/event/SceneColorEvent.h"
#include "anki/event/MainRendererPpsHdrEvent.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/Material.h"
#include "anki/core/ThreadPool.h"
#include "anki/core/Timestamp.h"
#include "anki/core/NativeWindow.h"
#include "anki/util/Functions.h"

using namespace anki;

ModelNode* horse;
PerspectiveCamera* cam;
NativeWindow* win;

//==============================================================================
void init()
{
	ANKI_LOGI("Other init...");

	Scene& scene = SceneSingleton::get();

#if 0
	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);
#endif

	// camera
	cam = new PerspectiveCamera("main-camera", &scene,
		Movable::MF_NONE, nullptr);
	const float ang = 45.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 500.0);
	cam->setLocalTransform(Transform(Vec3(100.0, 5.0, 8.0),
		Mat3(Euler(toRad(-10.0), toRad(90.0), toRad(0.0))),
		1.0));
	scene.setActiveCamera(cam);

	// camera 2
	PerspectiveCamera* pcam = new PerspectiveCamera("camera1", &scene,
		Movable::MF_NONE, nullptr);
	pcam->setAll(
		MainRendererSingleton::get().getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 200.0);
	pcam->setLocalTransform(Transform(Vec3(100.0, 3.0, 8.0),
		Mat3(Axisang(toRad(90.0), Vec3(0, 1, 0))),
		1.0));

	// lights
	Vec3 lpos(-100.0, 0.0, -50.0);
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			std::string name = "plight" + std::to_string(i) + std::to_string(j);

			PointLight* point = new PointLight(name.c_str(), &scene,
				Movable::MF_NONE, nullptr);
			point->setRadius(2.0);
			point->setDiffuseColor(Vec4(randFloat(3.0), randFloat(3.0), randFloat(3.0), 0.0));
			point->setSpecularColor(Vec4(randFloat(3.0), randFloat(3.0), randFloat(3.0), 0.0));
			point->setLocalTranslation(lpos);

			lpos.z() += 10.0;
		}

		lpos.x() += 4.0;
		lpos.z() = -50;
	}

#if 1
	SpotLight* spot = new SpotLight("spot0", &scene, Movable::MF_NONE, nullptr);
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec3(1.3, 4.3, 3.0),
		Mat3::getIdentity(), 1.0));
	spot->setDiffuseColor(Vec4(2.0));
	spot->setSpecularColor(Vec4(-1.0));
	spot->loadTexture("gfx/lights/flashlight.tga");
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);


	spot = new SpotLight("spot1", &scene, Movable::MF_NONE, nullptr);
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec3(5.3, 4.3, 3.0),
		Mat3::getIdentity(), 1.0));
	spot->setDiffuseColor(Vec4(3.0, 0.0, 0.0, 0.0));
	spot->setSpecularColor(Vec4(3.0, 3.0, 0.0, 0.0));
	spot->loadTexture("gfx/lights/flashlight.tga");
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);
#endif

	/*PointLight* point = new PointLight("point0", &scene, Movable::MF_NONE,
		nullptr);
	point->setRadius(3.0);
	point->setDiffuseColor(Vec4(1.0, 0.0, 0.0, 0.0));
	point->setSpecularColor(Vec4(0.0, 0.0, 1.0, 0.0));

	PointLight* point1 = new PointLight("point1", &scene, Movable::MF_NONE,
		nullptr);
	point1->setRadius(3.0);
	point1->setDiffuseColor(Vec4(2.0, 2.0, 2.0, 0.0));
	point1->setSpecularColor(Vec4(3.0, 3.0, 0.0, 0.0));
	point1->setLocalTranslation(Vec3(-3.0, 2.0, 0.0));*/

	// horse
	horse = new ModelNode("meshes/horse/horse.mdl", "horse", &scene,
		Movable::MF_NONE, nullptr);
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(),
		1.0));

#if 1
	// Sponza
	ModelNode* sponzaModel = new ModelNode(
		"maps/sponza-crytek/sponza_crytek.mdl",
		"sponza", &scene, Movable::MF_NONE, nullptr);

	sponzaModel->setLocalScale(0.1);
#endif

	// Sectors
	scene.sectors.push_back(new Sector(Aabb(Vec3(-10.0), Vec3(10.0))));
}

//==============================================================================
/// The func pools the stdinListener for string in the console, if
/// there are any it executes them with scriptingEngine
void execStdinScpripts()
{
	while(1)
	{
		std::string cmd = StdinListenerSingleton::get().getLine();

		if(cmd.length() < 1)
		{
			break;
		}

		try
		{
			ScriptManagerSingleton::get().evalString(cmd.c_str());
		}
		catch(Exception& e)
		{
			ANKI_LOGE(e.what());
		}
	}
}

//==============================================================================
void mainLoopExtra()
{
	InputSingleton::get().handleEvents();

	float dist = 0.2;
	float ang = toRad(3.0);
	float scale = 0.01;

	// move the camera
	static Movable* mover = SceneSingleton::get().getActiveCamera().getMovable();
	Input& in = InputSingleton::get();

	if(in.getKey(KC_1))
	{
		mover = &SceneSingleton::get().getActiveCamera();
	}
	if(in.getKey(KC_2))
	{
		mover = SceneSingleton::get().findSceneNode("horse")->getMovable();
	}
	if(in.getKey(KC_3))
	{
		mover = SceneSingleton::get().findSceneNode("spot0")->getMovable();
	}
	if(in.getKey(KC_4))
	{
		mover = SceneSingleton::get().findSceneNode("spot1")->getMovable();
	}
	/*if(in.getKey(KC_5))
	{
		mover = SceneSingleton::get().findSceneNode("point1")->getMovable();
	}*/
	if(in.getKey(KC_6))
	{
		mover = SceneSingleton::get().findSceneNode("camera1")->getMovable();
		mover->setLocalTransform(cam->getLocalTransform());
	}

	if(in.getKey(KC_L) == 1)
	{
		Light* l = SceneSingleton::get().findSceneNode("point1")->getLight();
		static_cast<PointLight*>(l)->setRadius(10.0);
	}

	if(in.getKey(KC_P) == 1)
	{
		MainRendererSingleton::get().getPps().getHdr().setExposure(20);
	}

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
	if(in.getKey(KC_PAGEUP))
	{
		mover->scale(scale);
	}
	if(in.getKey(KC_PAGEDOWN))
	{
		mover->scale(-scale);
	}

	execStdinScpripts();
}

//==============================================================================
void mainLoop()
{
	ANKI_LOGI("Entering main loop");

	HighRezTimer mainLoopTimer;
	mainLoopTimer.start();
	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	while(1)
	{
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		//
		mainLoopExtra();
		SceneSingleton::get().update(prevUpdateTime, crntTime);
		EventManagerSingleton::get().updateAllEvents(prevUpdateTime, crntTime);
		MainRendererSingleton::get().render(SceneSingleton::get());

		if(InputSingleton::get().getKey(KC_ESCAPE))
		{
			break;
		}

		//AppSingleton::get().swapBuffers();
		win->swapBuffers();

		// Sleep
		//
#if 1
		timer.stop();
		if(timer.getElapsedTime() < AppSingleton::get().getTimerTick())
		{
			HighRezTimer::sleep(AppSingleton::get().getTimerTick()
				- timer.getElapsedTime());
		}
#else
		if(MainRendererSingleton::get().getFramesCount() == 1000)
		{
			break;
		}
#endif
		Timestamp::increaseTimestamp();
	}

	ANKI_LOGI("Exiting main loop (" << mainLoopTimer.getElapsedTime()
		<< " sec)");
}

//==============================================================================
// initSubsystems                                                              =
//==============================================================================
void initSubsystems(int argc, char* argv[])
{
#if ANKI_GL == ANKI_GL_DESKTOP
	U32 glmajor = 3;
	U32 glminor = 3;
#else
	U32 glmajor = 3;
	U32 glminor = 0;
#endif

	// App
	AppSingleton::get().init(argc, argv);

	// Window
	NativeWindowInitializer nwinit;
	nwinit.width = 1280;
	nwinit.height = 720;
	nwinit.majorVersion = glmajor;
	nwinit.minorVersion = glminor;
	nwinit.depthBits = 0;
	nwinit.stencilBits = 0;
	win = new NativeWindow;	
	win->create(nwinit);

	// GL stuff
	GlStateCommonSingleton::get().init(glmajor, glminor);

	// Input
	InputSingleton::get().init(win);

	// Main renderer
	RendererInitializer initializer;
	initializer.ms.ez.enabled = true;
	initializer.dbg.enabled = false;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = false;
	initializer.is.sm.resolution = 512;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.25;
	initializer.pps.hdr.blurringDist = 0.1;
	initializer.pps.hdr.blurringIterationsCount = 2;
	initializer.pps.hdr.exposure = 5.0;
	initializer.pps.ssao.blurringIterationsNum = 4;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.renderingQuality = 0.3;
	initializer.pps.bl.enabled = true;
	initializer.pps.bl.blurringIterationsNum = 2;
	initializer.pps.bl.sideBlurFactor = 1.0;
	initializer.mainRendererQuality = 1.0;
	initializer.width = nwinit.width;
	initializer.height = nwinit.height;

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ThreadPoolSingleton::get().init(4);
}

//==============================================================================
int main(int argc, char* argv[])
{
	int exitCode;

	try
	{
		initSubsystems(argc, argv);
		init();

		mainLoop();

		ANKI_LOGI("Exiting...");
		AppSingleton::get().quit(EXIT_SUCCESS);
		exitCode = 0;
	}
	catch(std::exception& e)
	{
		std::cerr << "Aborting: " << e.what() << std::endl;
		exitCode = 1;
	}
	ANKI_LOGI("Bye!!");
	return exitCode;
}

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "anki/input/Input.h"
#include "anki/scene/Camera.h"
#include "anki/math/Math.h"
#include "anki/renderer/Renderer.h"
#include "anki/ui/UiPainter.h"
#include "anki/core/App.h"
#include "anki/resource/Mesh.h"
#include "anki/scene/Light.h"
#include "anki/resource/Material.h"
#include "anki/scene/Scene.h"
#include "anki/resource/SkelAnim.h"
#include "anki/misc/Parser.h"
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
#include "anki/ui/UiFtFontLoader.h"
#include "anki/ui/UiFont.h"
#include "anki/event/EventManager.h"
#include "anki/event/SceneColorEvent.h"
#include "anki/event/MainRendererPpsHdrEvent.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/Material.h"
#include "anki/core/ParallelManager.h"
#include "anki/core/Timestamp.h"

using namespace anki;

UiPainter* painter;
ModelNode* horse;
PerspectiveCamera* cam;

//==============================================================================
void init()
{
	ANKI_LOGI("Other init...");

	Scene& scene = SceneSingleton::get();

	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);

	// camera
	cam = new PerspectiveCamera("main-camera", &scene,
		Movable::MF_NONE, nullptr);
	const float ang = 45.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * Math::toRad(ang),
		Math::toRad(ang), 0.5, 200.0);
	cam->setLocalTransform(Transform(Vec3(100.0, 3.0, 8.0),
		Mat3(Axisang(Math::toRad(90), Vec3(0, 1, 0))),
		1.0));
	scene.setActiveCamera(cam);

	// camera 2
	PerspectiveCamera* pcam = new PerspectiveCamera("camera1", &scene,
		Movable::MF_NONE, nullptr);
	pcam->setAll(
		MainRendererSingleton::get().getAspectRatio() * Math::toRad(ang),
		Math::toRad(ang), 0.5, 200.0);
	pcam->setLocalTransform(Transform(Vec3(100.0, 3.0, 8.0),
		Mat3(Axisang(Math::toRad(90), Vec3(0, 1, 0))),
		1.0));

	// lights
	Vec3 lpos(-100.0, 0.0, 0.0);
	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 1; j++)
		{
			std::string name = "plight" + std::to_string(i) + std::to_string(j);

			PointLight* point = new PointLight(name.c_str(), &scene,
				Movable::MF_NONE, nullptr);
			point->setRadius(2.0);
			point->setDiffuseColor(Vec4(1.0, 0.0, 0.0, 0.0));
			point->setSpecularColor(Vec4(0.0, 0.0, 1.0, 0.0));
			point->setLocalTranslation(lpos);

			lpos.z() += 2.0;
		}

		lpos.x() += 2.0;
		lpos.z() = 0;
	}


	/*SpotLight* spot = new SpotLight("spot0", &scene, Movable::MF_NONE, nullptr);
	spot->setFov(Math::toRad(45.0));
	spot->setLocalTransform(Transform(Vec3(1.3, 4.3, 3.0),
		Mat3(Euler(Math::toRad(-20), Math::toRad(20), 0.0)), 1.0));
	spot->setDiffuseColor(Vec4(4.0));
	spot->setSpecularColor(Vec4(1.0));
	spot->loadTexture("gfx/lights/flashlight.tga");
	spot->setDistance(20.0);
	spot->setShadowEnabled(true);

	PointLight* point = new PointLight("point0", &scene, Movable::MF_NONE,
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

	// Sponza
	ModelNode* sponzaModel = new ModelNode(
		"/home/godlike/src/anki/maps/sponza-crytek/sponza_crytek.mdl",
		"sponza", &scene, Movable::MF_NONE, nullptr);

	sponzaModel->setLocalScale(0.1);

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
	float ang = Math::toRad(3.0);
	float scale = 0.01;

	// move the camera
	static Movable* mover = SceneSingleton::get().getActiveCamera().getMovable();
	Input& in = InputSingleton::get();

	SceneSingleton::get().setAmbientColor(Vec3(0.2));

	if(in.getKey(SDL_SCANCODE_1))
	{
		mover = &SceneSingleton::get().getActiveCamera();
	}
	if(in.getKey(SDL_SCANCODE_2))
	{
		mover = SceneSingleton::get().findSceneNode("horse")->getMovable();
	}
	/*if(in.getKey(SDL_SCANCODE_3))
	{
		mover = SceneSingleton::get().findSceneNode("spot0")->getMovable();
	}
	if(in.getKey(SDL_SCANCODE_4))
	{
		mover = SceneSingleton::get().findSceneNode("point0")->getMovable();
	}
	if(in.getKey(SDL_SCANCODE_5))
	{
		mover = SceneSingleton::get().findSceneNode("point1")->getMovable();
	}*/
	if(in.getKey(SDL_SCANCODE_6))
	{
		mover = SceneSingleton::get().findSceneNode("camera1")->getMovable();
		mover->setLocalTransform(cam->getLocalTransform());
	}

	if(in.getKey(SDL_SCANCODE_L) == 1)
	{
		Light* l = SceneSingleton::get().findSceneNode("point1")->getLight();
		static_cast<PointLight*>(l)->setRadius(10.0);
	}

	if(in.getKey(SDL_SCANCODE_UP)) mover->rotateLocalX(ang);
	if(in.getKey(SDL_SCANCODE_DOWN)) mover->rotateLocalX(-ang);
	if(in.getKey(SDL_SCANCODE_LEFT)) mover->rotateLocalY(ang);
	if(in.getKey(SDL_SCANCODE_RIGHT)) mover->rotateLocalY(-ang);

	if(in.getKey(SDL_SCANCODE_A)) mover->moveLocalX(-dist);
	if(in.getKey(SDL_SCANCODE_D)) mover->moveLocalX(dist);
	if(in.getKey(SDL_SCANCODE_LSHIFT)) mover->moveLocalY(dist);
	if(in.getKey(SDL_SCANCODE_SPACE)) mover->moveLocalY(-dist);
	if(in.getKey(SDL_SCANCODE_W)) mover->moveLocalZ(-dist);
	if(in.getKey(SDL_SCANCODE_S)) mover->moveLocalZ(dist);
	if(in.getKey(SDL_SCANCODE_Q)) mover->rotateLocalZ(ang);
	if(in.getKey(SDL_SCANCODE_E)) mover->rotateLocalZ(-ang);
	if(in.getKey(SDL_SCANCODE_PAGEUP))
	{
		mover->scale(scale);
	}
	if(in.getKey(SDL_SCANCODE_PAGEDOWN))
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

		if(InputSingleton::get().getKey(SDL_SCANCODE_ESCAPE))
		{
			break;
		}

		AppSingleton::get().swapBuffers();

		// Sleep
		//
#if 1
		timer.stop();
		if(timer.getElapsedTime() < AppSingleton::get().getTimerTick())
		{
			SDL_Delay((AppSingleton::get().getTimerTick()
				- timer.getElapsedTime()) * 1000.0);
		}
#else
		if(MainRendererSingleton::get().getFramesCount() == 100)
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
	// App
	AppSingleton::get().init(argc, argv);

	// Main renderer
	RendererInitializer initializer;
	initializer.ms.ez.enabled = true;
	initializer.dbg.enabled = false;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = true;
	initializer.is.sm.resolution = 1024;
	initializer.is.sm.level0Distance = 3.0;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.25;
	initializer.pps.hdr.blurringDist = 1.0;
	initializer.pps.hdr.blurringIterationsNum = 2;
	initializer.pps.hdr.exposure = 4.0;
	initializer.pps.ssao.blurringIterationsNum = 4;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.renderingQuality = 0.3;
	initializer.pps.bl.enabled = true;
	initializer.pps.bl.blurringIterationsNum = 2;
	initializer.pps.bl.sideBlurFactor = 1.0;
	initializer.mainRendererQuality = 1.0;

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ParallelManagerSingleton::get().init(4);
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

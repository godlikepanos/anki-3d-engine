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
#include "anki/resource/LightRsrc.h"
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

using namespace anki;

UiPainter* painter;

//==============================================================================
void init()
{
	ANKI_LOGI("Other init...");

	Scene& scene = SceneSingleton::get();

	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);

	// camera
	PerspectiveCamera* cam = new PerspectiveCamera("main-camera", &scene,
		Movable::MF_NONE, nullptr);
	const float ang = 70.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * Math::toRad(ang),
		Math::toRad(ang), 0.5, 200.0);
	cam->moveLocalY(3.0);
	cam->moveLocalZ(5.7);
	cam->moveLocalX(-0.3);
	scene.setActiveCamera(cam);

	// lights

	SpotLight* spot = new SpotLight("spot0", &scene, Movable::MF_NONE, nullptr);
	spot->setFov(Math::toRad(45.0));
	spot->setLocalTransform(Transform(Vec3(1.3, 4.3, 3.0),
		Mat3(Euler(Math::toRad(-20), Math::toRad(20), 0.0)), 1.0));
	spot->setColor(Vec4(4.0));
	spot->setSpecularColor(Vec4(1.0));
	spot->loadTexture("gfx/lights/flashlight.tga");
	spot->setDistance(20.0);
	spot->setShadowEnabled(true);

	// horse
	ModelNode* horse = new ModelNode("meshes/horse/horse.mdl", "horse", &scene,
		Movable::MF_NONE, nullptr);
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(),
		1.0));
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


	if(in.getKey(SDL_SCANCODE_1))
	{
		mover = &SceneSingleton::get().getActiveCamera();
	}

	if(in.getKey(SDL_SCANCODE_A)) mover->moveLocalX(-dist);
	if(in.getKey(SDL_SCANCODE_D)) mover->moveLocalX(dist);
	if(in.getKey(SDL_SCANCODE_LSHIFT)) mover->moveLocalY(dist);
	if(in.getKey(SDL_SCANCODE_SPACE)) mover->moveLocalY(-dist);
	if(in.getKey(SDL_SCANCODE_W)) mover->moveLocalZ(-dist);
	if(in.getKey(SDL_SCANCODE_S)) mover->moveLocalZ(dist);
	if(in.getKey(SDL_SCANCODE_Q)) mover->rotateLocalZ(ang);
	if(in.getKey(SDL_SCANCODE_E)) mover->rotateLocalZ(-ang);
	if(in.getKey(SDL_SCANCODE_PAGEUP)) mover->getLocalTransform().getScale() += scale ;
	if(in.getKey(SDL_SCANCODE_PAGEDOWN)) mover->getLocalTransform().getScale() -= scale ;
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
		SceneSingleton::get().update(prevUpdateTime, crntTime,
			MainRendererSingleton::get().getFramesCount());
		EventManagerSingleton::get().updateAllEvents(prevUpdateTime, crntTime);
		MainRendererSingleton::get().render(SceneSingleton::get());

		if(InputSingleton::get().getKey(SDL_SCANCODE_ESCAPE))
		{
			break;
		}

		AppSingleton::get().swapBuffers();
	}

	ANKI_LOGI("Exiting main loop (" << mainLoopTimer.getElapsedTime() << " sec)");
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
	initializer.dbg.enabled = true;
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
		//abort();
		exitCode = 1;
	}

	return exitCode;
}

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
#include "anki/core/Timestamp.h"
#include "anki/core/NativeWindow.h"
#include "anki/Scene.h"
#include "anki/event/LightEvent.h"
#include "anki/event/MovableEvent.h"
#include "anki/core/Counters.h"

using namespace anki;

ModelNode* horse;
PerspectiveCamera* cam;
NativeWindow* win;

//==============================================================================
void initPhysics()
{
	SceneGraph& scene = SceneGraphSingleton::get();

	scene.getPhysics().setDebugDrawer(
		new PhysicsDebugDrawer(
			&MainRendererSingleton::get().getDbg().getDebugDrawer()));

	btCollisionShape* groundShape = new btBoxShape(
	    btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

	Transform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 0));

	RigidBody::Initializer init;
	init.mass = 0.0;
	init.shape = groundShape;
	init.startTrf = groundTransform;
	init.group = PhysWorld::CG_MAP;
	init.mask = PhysWorld::CG_ALL;

	new RigidBody(&SceneGraphSingleton::get().getPhysics(), init);

#if 1
	btCollisionShape* colShape = new btBoxShape(
	    btVector3(1, 1, 1));

	init.startTrf.setOrigin(Vec3(0.0, 15.0, 0.0));
	init.mass = 20;
	init.shape = colShape;
	init.group = PhysWorld::CG_PARTICLE;
	init.mask = PhysWorld::CG_MAP | PhysWorld::CG_PARTICLE;

	const I ARRAY_SIZE_X = 5;
	const I ARRAY_SIZE_Y = 5;
	const I ARRAY_SIZE_Z = 5;
	const I START_POS_X = -5;
	const I START_POS_Y = 35;
	const I START_POS_Z = -3;

	float start_x = START_POS_X - ARRAY_SIZE_X / 2;
	float start_y = START_POS_Y;
	float start_z = START_POS_Z - ARRAY_SIZE_Z / 2;

	for(I k = 0; k < ARRAY_SIZE_Y; k++)
	{
		for(I i = 0; i < ARRAY_SIZE_X; i++)
		{
			for(I j = 0; j < ARRAY_SIZE_Z; j++)
			{
				std::string name = std::string("crate0") + std::to_string(i)
					+ std::to_string(j) + std::to_string(k);

				ModelNode* mnode = new ModelNode(
					name.c_str(), &SceneGraphSingleton::get(), nullptr,
					Movable::MF_NONE, "data/models/crate0/crate0.mdl");

				init.movable = mnode;
				ANKI_ASSERT(init.movable);

				Transform trf(
					Vec3(2.0 * i + start_x, 2.0 * k + start_y,
						2.0 * j + start_z), Mat3::getIdentity(), 1.0);

				init.startTrf = trf;

				new RigidBody(&SceneGraphSingleton::get().getPhysics(), init);
			}
		}
	}
#endif
}

//==============================================================================
void init()
{
	ANKI_LOGI("Other init...");

	SceneGraph& scene = SceneGraphSingleton::get();

#if 0
	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);
#endif

	// camera
	cam = new PerspectiveCamera("main-camera", &scene, nullptr,
		Movable::MF_NONE);
	const F32 ang = 45.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 500.0);
	cam->setLocalTransform(Transform(Vec3(18.0, 5.2, 0.0),
		Mat3(Euler(toRad(-10.0), toRad(90.0), toRad(0.0))),
		1.0));
	scene.setActiveCamera(cam);

	// lights
#if 1
	Vec3 lpos(-24.0, 0.1, -10.0);
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 10; j++)
		{
			std::string name = "plight" + std::to_string(i) + std::to_string(j);

			PointLight* point = new PointLight(name.c_str(), &scene, nullptr,
				Movable::MF_NONE);
			point->setRadius(0.5);
			point->setDiffuseColor(Vec4(randFloat(6.0) - 2.0, 
				randFloat(6.0) - 2.0, randFloat(6.0) - 2.0, 0.0));
			point->setSpecularColor(Vec4(randFloat(6.0) - 3.0, 
				randFloat(6.0) - 3.0, randFloat(6.0) - 3.0, 0.0));
			point->setLocalOrigin(lpos);

			lpos.z() += 2.0;
		}

		lpos.x() += 0.93;
		lpos.z() = -10;
	}
#endif

#if 1
	SpotLight* spot = new SpotLight("spot0", &scene, nullptr, Movable::MF_NONE);
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec3(8.27936, 5.86285, 1.85526),
		Mat3(Quat(-0.125117, 0.620465, 0.154831, 0.758544)), 1.0));
	spot->setDiffuseColor(Vec4(2.0));
	spot->setSpecularColor(Vec4(-1.0));
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);


	spot = new SpotLight("spot1", &scene, nullptr, Movable::MF_NONE);
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec3(5.3, 4.3, 3.0),
		Mat3::getIdentity(), 1.0));
	spot->setDiffuseColor(Vec4(3.0, 0.0, 0.0, 0.0));
	spot->setSpecularColor(Vec4(3.0, 3.0, 0.0, 0.0));
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);
#endif

#if 1
	// Vase point lights
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
			(i != 100) ? "data/textures/lens_flare/flares0.ankitex" : nullptr);
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
			Movable::MF_NONE, "data/particles/smoke.particles");
		pe->setLocalOrigin(lightPos);

		pe = new ParticleEmitter(
			("pef" + std::to_string(i)).c_str(), &scene, nullptr,
			Movable::MF_NONE, "data/particles/fire.particles");
		pe->setLocalOrigin(lightPos);
	}
#endif

#if 1
	// horse
	horse = new ModelNode("horse", &scene, nullptr,
		Movable::MF_NONE, "data/models/horse/horse.mdl");
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(),
		0.7));

	// barrel
	ModelNode* redBarrel = new ModelNode(
		"red_barrel", &scene, nullptr, Movable::MF_NONE, 
		"data/models/red_barrel/red_barrel.mdl");
	redBarrel->setLocalTransform(Transform(Vec3(+2, 0, 0), Mat3::getIdentity(),
		0.7));
#endif

#if 0
	StaticGeometryNode* sponzaModel = new StaticGeometryNode(
		//"data/maps/sponza/sponza_no_bmeshes.mdl",
		//"data/maps/sponza/sponza.mdl",
		"sponza", &scene, "data/maps/sponza/static_geometry.mdl");

	(void)sponzaModel;
#endif
	scene.load("data/maps/sponza/master.scene");

	//initPhysics();

	// Sectors
	SectorGroup& sgroup = scene.getSectorGroup();

	Sector* sectorA = sgroup.createNewSector(
		Aabb(Vec3(-38, -3, -20), Vec3(38, 27, 20)));
	Sector* sectorB = sgroup.createNewSector(Aabb(Vec3(-5), Vec3(5)));

	sgroup.createNewPortal(sectorA, sectorB, Obb(Vec3(0.0, 3.0, 0.0),
		Mat3::getIdentity(), Vec3(1.0, 2.0, 2.0)));

	Sector* sectorC = sgroup.createNewSector(
		Aabb(Vec3(-30, -10, -35), Vec3(30, 10, -25)));

	sgroup.createNewPortal(sectorA, sectorC, Obb(Vec3(-1.1, 2.0, -11.0),
		Mat3::getIdentity(), Vec3(1.3, 1.8, 0.5)));

	// Path
	/*Path* path = new Path("todo", "path", &scene, Movable::MF_NONE, nullptr);
	(void)path;

	const F32 distPerSec = 2.0;
	scene.getEventManager().newFollowPathEvent(-1.0, 
		path->getDistance() / distPerSec, 
		cam, path, distPerSec);*/
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
	F32 dist = 0.2;
	F32 ang = toRad(3.0);
	F32 scale = 0.01;
	F32 mouseSensivity = 9.0;

	// move the camera
	static Movable* mover = SceneGraphSingleton::get().getActiveCamera().getMovable();
	Input& in = InputSingleton::get();

	if(in.getKey(KC_1))
	{
		mover = &SceneGraphSingleton::get().getActiveCamera();
	}
	if(in.getKey(KC_2))
	{
		mover = SceneGraphSingleton::get().findSceneNode("horse").getMovable();
	}
	if(in.getKey(KC_3))
	{
		mover = SceneGraphSingleton::get().findSceneNode("spot0").getMovable();
	}
	if(in.getKey(KC_4))
	{
		mover = SceneGraphSingleton::get().findSceneNode("spot1").getMovable();
	}
	if(in.getKey(KC_5))
	{
		mover = SceneGraphSingleton::get().findSceneNode("pe").getMovable();
	}
	if(in.getKey(KC_6))
	{
		mover = SceneGraphSingleton::get().findSceneNode("vase_plight0").getMovable();
	}
	if(in.getKey(KC_7))
	{
		mover = SceneGraphSingleton::get().findSceneNode("red_barrel").getMovable();
	}

	if(in.getKey(KC_L) == 1)
	{
		Light* l = SceneGraphSingleton::get().findSceneNode("point1").getLight();
		static_cast<PointLight*>(l)->setRadius(10.0);
	}

	if(in.getKey(KC_F1) == 1)
	{
		MainRendererSingleton::get().getDbg().setEnabled(
			!MainRendererSingleton::get().getDbg().getEnabled());
	}
	if(in.getKey(KC_F2) == 1)
	{
		MainRendererSingleton::get().getDbg().switchBits(
			Dbg::DF_SPATIAL);
	}
	if(in.getKey(KC_F3) == 1)
	{
		MainRendererSingleton::get().getDbg().switchBits(
			Dbg::DF_PHYSICS);
	}
	if(in.getKey(KC_F4) == 1)
	{
		MainRendererSingleton::get().getDbg().switchBits(
			Dbg::DF_SECTOR);
	}
	if(in.getKey(KC_F5) == 1)
	{
		MainRendererSingleton::get().getDbg().switchBits(
			Dbg::DF_OCTREE);
	}
	if(in.getKey(KC_F6) == 1)
	{
		MainRendererSingleton::get().getDbg().switchDepthTestEnabled();
	}
	if(in.getKey(KC_F12) == 1)
	{
		MainRendererSingleton::get().takeScreenshot("screenshot.tga");
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
	if(in.getKey(KC_P) == 1)
	{
		std::cout << "{Vec3(" << mover->getWorldTransform().getOrigin()
			<< "), Quat(" << Quat(mover->getWorldTransform().getRotation())
			<< ")}," << std::endl;
	}


	if(in.getMousePosition() != Vec2(0.0))
	{
		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity *
			MainRendererSingleton::get().getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}

	execStdinScpripts();
}

//==============================================================================
void mainLoop()
{
	ANKI_LOGI("Entering main loop");

	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	ANKI_COUNTER_START_TIMER(C_FPS);

	while(1)
	{
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		//
		InputSingleton::get().handleEvents();
		InputSingleton::get().moveMouse(Vec2(0.0));
		mainLoopExtra();
		SceneGraphSingleton::get().update(
			prevUpdateTime, crntTime, MainRendererSingleton::get());
		//EventManagerSingleton::get().updateAllEvents(prevUpdateTime, crntTime);
		MainRendererSingleton::get().render(SceneGraphSingleton::get());

		if(InputSingleton::get().getKey(KC_ESCAPE))
		{
			break;
		}

		win->swapBuffers();
		ANKI_COUNTERS_RESOLVE_FRAME();

		// Sleep
		//
#if 0
		timer.stop();
		if(timer.getElapsedTime() < AppSingleton::get().getTimerTick())
		{
			HighRezTimer::sleep(AppSingleton::get().getTimerTick()
				- timer.getElapsedTime());
		}
#else
		if(MainRendererSingleton::get().getFramesCount() == 2000)
		{
			break;
		}
#endif
		increaseGlobTimestamp();
	}

#if 0
	MainRendererSingleton::get().takeScreenshot("screenshot.tga");
#endif

	ANKI_COUNTER_STOP_TIMER_INC(C_FPS);

	ANKI_COUNTERS_FLUSH();
}

//==============================================================================
// initSubsystems                                                              =
//==============================================================================
void initSubsystems(int argc, char* argv[])
{
#if ANKI_GL == ANKI_GL_DESKTOP
	U32 glmajor = 4;
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
	nwinit.fullscreenDesktopRez = true;
	nwinit.debugContext = false;
	win = new NativeWindow;	
	win->create(nwinit);

	// GL stuff
	GlStateCommonSingleton::get().init(glmajor, glminor, nwinit.debugContext);

	// Input
	InputSingleton::get().init(win);
	InputSingleton::get().hideCursor(true);

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
	initializer.width = win->getWidth();
	initializer.height = win->getHeight();
	initializer.lodDistance = 20.0;

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ThreadPoolSingleton::get().init(getCpuCoresCount());
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
		ANKI_LOGE("Aborting: " << e.what());
		exitCode = 1;
	}
	ANKI_LOGI("Bye!!");
	return exitCode;
}

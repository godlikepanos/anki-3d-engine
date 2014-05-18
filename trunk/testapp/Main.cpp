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
#include "anki/resource/Material.h"
#include "anki/core/Timestamp.h"
#include "anki/core/NativeWindow.h"
#include "anki/Scene.h"
#include "anki/event/LightEvent.h"
#include "anki/event/MoveEvent.h"
#include "anki/core/Counters.h"

using namespace anki;

ModelNode* horse;
PerspectiveCamera* cam;
NativeWindow* win;
HeapAllocator<U8> globAlloc;

//==============================================================================
void initPhysics()
{
	SceneGraph& scene = SceneGraphSingleton::get();

	btCollisionShape* groundShape = new btBoxShape(
	    btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

	Transform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0, -50, 0));

	RigidBody::Initializer init;
	init.mass = 0.0;
	init.shape = groundShape;
	init.startTrf = groundTransform;
	init.group = PhysicsWorld::CG_MAP;
	init.mask = PhysicsWorld::CG_ALL;

	new RigidBody(&SceneGraphSingleton::get().getPhysics(), init);

#if 0
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
					MoveComponent::MF_NONE, "models/crate0/crate0.mdl");

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
	scene.setAmbientColor(Vec4(0.1, 0.05, 0.05, 0.0) * 3);

#if 0
	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);
#endif

	// camera
	cam = scene.newSceneNode<PerspectiveCamera>("main-camera");
	const F32 ang = 45.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 500.0);
	cam->setLocalTransform(Transform(Vec3(17.0, 5.2, 0.0),
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

			PointLight* point = scene.newSceneNode<PointLight>(name.c_str());
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
	SpotLight* spot = scene.newSceneNode<SpotLight>("spot0");
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec3(8.27936, 5.86285, 1.85526),
		Mat3(Quat(-0.125117, 0.620465, 0.154831, 0.758544)), 1.0));
	spot->setDiffuseColor(Vec4(2.0));
	spot->setSpecularColor(Vec4(-1.0));
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);


	spot = scene.newSceneNode<SpotLight>("spot1");
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

		PointLight* point = scene.newSceneNode<PointLight>(
			("vase_plight" + std::to_string(i)).c_str());

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

		ParticleEmitter* pe;
		/**/

		if(i == 0)
		{
			/*scene.newSceneNode(pe, "pefire", "particles/fire.ankipart");
			pe->setLocalOrigin(lightPos);

			scene.newSceneNode(pe, "pesmoke", "particles/smoke.ankipart");
			pe->setLocalOrigin(lightPos);*/
		}
		/*else
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
			scene.newSceneNode(pe, ("pesparks" + std::to_string(i)).c_str(), 
				"particles/sparks.ankipart");
			pe->setLocalOrigin(lightPos);
		}*/
	}
#endif

#if 1
	// horse
	horse = scene.newSceneNode<ModelNode>("horse", "models/horse/horse.ankimdl");
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(),
		0.7));


	horse = scene.newSceneNode<ModelNode>("crate", "models/crate0/crate0.ankimdl");
	horse->setLocalTransform(Transform(Vec3(2, 10.0, 0), Mat3::getIdentity(),
		1.0));

	// barrel
	/*ModelNode* redBarrel = new ModelNode(
		"red_barrel", &scene, nullptr, MoveComponent::MF_NONE, 
		"models/red_barrel/red_barrel.mdl");
	redBarrel->setLocalTransform(Transform(Vec3(+2, 0, 0), Mat3::getIdentity(),
		0.7));*/
#endif

#if 0
	StaticGeometryNode* sponzaModel = new StaticGeometryNode(
		//"data/maps/sponza/sponza_no_bmeshes.mdl",
		//"data/maps/sponza/sponza.mdl",
		"sponza", &scene, "data/maps/sponza/static_geometry.mdl");

	(void)sponzaModel;
#endif
	//scene.load("maps/sponza/master.ankiscene");
	{
		String str;
		File file(ANKI_R("maps/sponza/scene.lua"), File::OpenFlag::READ);
		file.readAllText(str);
		ScriptManagerSingleton::get().evalString(str.c_str());
	}

	initPhysics();

	// Sectors
#if 0
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
#endif

	// Path
	/*Path* path = new Path("todo", "path", &scene, MoveComponent::MF_NONE, nullptr);
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
	static MoveComponent* mover = 
		&SceneGraphSingleton::get().getActiveCamera().getComponent<MoveComponent>();
	Input& in = InputSingleton::get();

	if(in.getKey(KC_1))
	{
		mover = &SceneGraphSingleton::get().getActiveCamera();
	}
	if(in.getKey(KC_2))
	{
		mover = &SceneGraphSingleton::get().findSceneNode("horse").getComponent<MoveComponent>();
	}
	if(in.getKey(KC_3))
	{
		mover = &SceneGraphSingleton::get().findSceneNode("spot0").getComponent<MoveComponent>();
	}
	if(in.getKey(KC_4))
	{
		mover = &SceneGraphSingleton::get().findSceneNode("spot1").getComponent<MoveComponent>();
	}
	if(in.getKey(KC_5))
	{
		mover = &SceneGraphSingleton::get().findSceneNode("pe").getComponent<MoveComponent>();
	}
	if(in.getKey(KC_6))
	{
		mover = &SceneGraphSingleton::get().findSceneNode("vase_plight0").getComponent<MoveComponent>();
	}
	if(in.getKey(KC_7))
	{
		mover = &SceneGraphSingleton::get().findSceneNode("red_barrel").getComponent<MoveComponent>();
	}

	/*if(in.getKey(KC_L) == 1)
	{
		SceneNode& l = 
			SceneGraphSingleton::get().findSceneNode("crate");
		
		Transform trf;
		trf.setIdentity();
		trf.getOrigin().y() = 20.0;
		l.getComponent<MoveComponent>().setLocalTransform(trf);
	}*/

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
		std::cout << "{Vec3(" 
			<< mover->getWorldTransform().getOrigin().toString()
			<< "), Quat(" 
			<< Quat(mover->getWorldTransform().getRotation()).toString()
			<< ")}," << std::endl;
	}

	if(in.getKey(KC_L) == 1)
	{
		try
		{
			ScriptManagerSingleton::get().evalString(
R"(scene = SceneGraphSingleton.get()
node = scene:tryFindSceneNode("horse")
if Anki.userDataValid(node) == 1 then
	print("valid")
else 
	print("invalid")
end)");
		}
		catch(Exception& e)
		{
			ANKI_LOGE(e.what());
		}
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

	ANKI_COUNTER_START_TIMER(FPS);

	while(1)
	{
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		//
		InputSingleton::get().handleEvents();
		mainLoopExtra();

		SceneGraphSingleton::get().update(
			prevUpdateTime, crntTime, MainRendererSingleton::get());
		//EventManagerSingleton::get().updateAllEvents(prevUpdateTime, crntTime);
		MainRendererSingleton::get().render(SceneGraphSingleton::get());

		if(InputSingleton::get().getKey(KC_ESCAPE))
		{
			break;
		}

		GlManagerSingleton::get().swapBuffers();
		ANKI_COUNTERS_RESOLVE_FRAME();

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
		if(MainRendererSingleton::get().getFramesCount() == 2000)
		{
			break;
		}
#endif
		increaseGlobTimestamp();
	}

	ANKI_COUNTER_STOP_TIMER_INC(FPS);

	ANKI_COUNTERS_FLUSH();

#if 0
	MainRendererSingleton::get().takeScreenshot("screenshot.tga");
#endif
}

//==============================================================================
// initSubsystems                                                              =
//==============================================================================

void makeCurrent(void* ctx)
{
	ANKI_ASSERT(ctx);
	win->contextMakeCurrent(ctx);
}

void swapBuffers(void* wind)
{
	NativeWindow* win = (NativeWindow*)wind;
	win->swapBuffers();
}

void initSubsystems(int argc, char* argv[])
{
#if ANKI_GL == ANKI_GL_DESKTOP
	U32 glmajor = 4;
	U32 glminor = 4;
#else
	U32 glmajor = 3;
	U32 glminor = 0;
#endif

	globAlloc = HeapAllocator<U8>(HeapMemoryPool(0));

	// Logger
	LoggerSingleton::get().init(
		Logger::InitFlags::WITH_SYSTEM_MESSAGE_HANDLER, globAlloc);

	// App
	AppSingleton::get().init();

	// Window
	NativeWindowInitializer nwinit;
	nwinit.m_width = 1280;
	nwinit.m_height = 720;
	nwinit.m_majorVersion = glmajor;
	nwinit.m_minorVersion = glminor;
	nwinit.m_depthBits = 0;
	nwinit.m_stencilBits = 0;
	nwinit.m_fullscreenDesktopRez = true;
	nwinit.m_debugContext = ANKI_DEBUG;
	win = new NativeWindow;	
	win->create(nwinit);
	void* context = win->getCurrentContext();
	win->contextMakeCurrent(nullptr);

	// GL stuff
	GlManagerSingleton::get().init(makeCurrent, context,
		swapBuffers, win, nwinit.m_debugContext);

	// Input
	InputSingleton::get().init(win);
	InputSingleton::get().lockCursor(true);
	InputSingleton::get().hideCursor(true);
	InputSingleton::get().moveCursor(Vec2(0.0));

	// Main renderer
	RendererInitializer initializer;
	initializer.set("ms.ez.enabled", false);
	initializer.set("ms.ez.maxObjectsToDraw", 100);
	initializer.set("dbg.enabled", false);
	initializer.set("is.sm.bilinearEnabled", true);
	initializer.set("is.groundLightEnabled", true);
	initializer.set("is.sm.enabled", true);
	initializer.set("is.sm.poissonEnabled", true);
	initializer.set("is.sm.resolution", 1024);
	initializer.set("pps.enabled", true);
	initializer.set("pps.hdr.enabled", true);
	initializer.set("pps.hdr.renderingQuality", 0.6);
	initializer.set("pps.hdr.blurringDist", 1.0);
	initializer.set("pps.hdr.blurringIterationsCount", 1);
	initializer.set("pps.hdr.exposure", 8.0);
	initializer.set("pps.hdr.samples", 9);
	initializer.set("pps.sslr.renderingQuality", 0.5);
	initializer.set("pps.ssao.blurringIterationsNum", 1);
	initializer.set("pps.ssao.enabled", true);
	initializer.set("pps.ssao.renderingQuality", 0.35);
	initializer.set("pps.bl.enabled", true);
	initializer.set("pps.bl.blurringIterationsNum", 2);
	initializer.set("pps.bl.sideBlurFactor", 1.0);
	initializer.set("pps.lf.enabled", true);
	initializer.set("pps.sharpen", true);
	initializer.set("renderingQuality", 1.0);
	initializer.set("width", win->getWidth());
	initializer.set("height", win->getHeight());
	initializer.set("lodDistance", 20.0);
	initializer.set("samples", 1);
	initializer.set("tessellation", false);
	initializer.set("tilesXCount", 16);
	initializer.set("tilesYCount", 16);

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ThreadpoolSingleton::get().init(getCpuCoresCount());
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
		ANKI_LOGE("Aborting: %s", e.what());
		exitCode = 1;
	}
	ANKI_LOGI("Bye!!");
	return exitCode;
}

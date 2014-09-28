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
#include "anki/renderer/Renderer.h"
#include "anki/renderer/MainRenderer.h"
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
#include "anki/event/AnimationEvent.h"
#include "anki/event/MoveEvent.h"
#include "anki/core/Counters.h"
#include "anki/core/Config.h"

using namespace anki;

App* app;

ModelNode* horse;
PerspectiveCamera* cam;


//==============================================================================
void init()
{
	ANKI_LOGI("Other init...");

	SceneGraph& scene = app->getSceneGraph();
	MainRenderer& renderer = app->getMainRenderer();
	ResourceManager& resources = app->getResourceManager();

	scene.setAmbientColor(Vec4(0.1, 0.05, 0.05, 0.0) * 3);

	if(getenv("PROFILE"))
	{
		app->setTimerTick(0.0);
	}

#if 0
	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);
#endif

	// camera
	cam = scene.newSceneNode<PerspectiveCamera>("main-camera");
	const F32 ang = 45.0;
	cam->setAll(
		renderer.getAspectRatio() * toRad(ang),
		toRad(ang), 0.5, 500.0);
	cam->setLocalTransform(Transform(Vec4(17.0, 5.2, 0.0, 0),
		Mat3x4(Euler(toRad(-10.0), toRad(90.0), toRad(0.0))),
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
			point->setLocalOrigin(lpos.xyz0());

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
	spot->setLocalTransform(Transform(Vec4(8.27936, 5.86285, 1.85526, 0.0),
		Mat3x4(Quat(-0.125117, 0.620465, 0.154831, 0.758544)), 1.0));
	spot->setDiffuseColor(Vec4(1.0));
	spot->setSpecularColor(Vec4(-1.0));
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);

#endif

#if 1
	spot = scene.newSceneNode<SpotLight>("spot1");
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec4(5.3, 4.3, 3.0, 0.0),
		Mat3x4::getIdentity(), 1.0));
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
		Vec4 lightPos = vaseLightPos[i].xyz0();

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
		moveData.m_posMin = Vec4(-0.5, 0.0, -0.5, 0);
		moveData.m_posMax = Vec4(0.5, 0.0, 0.5, 0);
		MoveEvent* mevent;
		scene.getEventManager().newEvent(mevent, 0.0, 2.0, point, moveData);
		mevent->enableBits(Event::EF_REANIMATE);

		ParticleEmitter* pe;
		/**/

		if(i == 0)
		{
			pe = scene.newSceneNode<ParticleEmitter>(
				"pefire", "particles/fire.ankipart");
			pe->setLocalOrigin(lightPos);

			pe = scene.newSceneNode<ParticleEmitter>(
				"pesmoke", "particles/smoke.ankipart");
			pe->setLocalOrigin(lightPos);
		}
		else
		{
			InstanceNode* instance;
			instance = scene.newSceneNode<InstanceNode>( 
				("pefire_inst" + std::to_string(i)).c_str());

			instance->setLocalOrigin(lightPos);

			SceneNode& sn = scene.findSceneNode("pefire");
			sn.addChild(instance);

			instance = scene.newSceneNode<InstanceNode>(
				("pesmoke_inst" + std::to_string(i)).c_str());

			instance->setLocalOrigin(lightPos);

			scene.findSceneNode("pesmoke").addChild(instance);
		}

		/*{
			scene.newSceneNode(pe, ("pesparks" + std::to_string(i)).c_str(), 
				"particles/sparks.ankipart");
			pe->setLocalOrigin(lightPos);
		}*/
	}
#endif

#if 1
	// horse
	horse = scene.newSceneNode<ModelNode>("horse", "models/horse/horse.ankimdl");
	horse->setLocalTransform(
		Transform(Vec4(-2, 0, 0, 0.0), Mat3x4::getIdentity(), 0.7));

	//horse = scene.newSceneNode<ModelNode>("crate", "models/crate0/crate0.ankimdl");
	//horse->setLocalTransform(Transform(Vec3(2, 10.0, 0), Mat3::getIdentity(),
	//	1.0));

	// barrel
	/*ModelNode* redBarrel = new ModelNode(
		"red_barrel", &scene, nullptr, MoveComponent::MF_NONE, 
		"models/red_barrel/red_barrel.mdl");
	redBarrel->setLocalTransform(Transform(Vec3(+2, 0, 0), Mat3::getIdentity(),
		0.7));*/
#endif

#if 1
	{
		String str(app->getAllocator());
		File file(
			resources.fixResourceFilename("maps/sponza/scene.lua").toCString(), 
			File::OpenFlag::READ);
		file.readAllText(str);
		app->getScriptManager().evalString(str.toCString());
	}
#endif

	/*AnimationResourcePointer anim;
	anim.load("maps/sponza/unnamed_0.ankianim");
	AnimationEvent* event;
	scene.getEventManager().newEvent(event, anim, cam);*/

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


#if 0
	horse = scene.newSceneNode<ModelNode>("shape0", 
		"models/collision_test/Cube_Material-material.ankimdl");
	horse->setLocalTransform(Transform(Vec4(0.0, 0, 0, 0), 
		Mat3x4::getIdentity(), 0.01));

	horse = scene.newSceneNode<ModelNode>("shape1", 
		"models/collision_test/Cube.001_Material_001-material.ankimdl");
	horse->setLocalTransform(Transform(Vec4(3.1, 2, 0.2, 0), 
		Mat3x4(Euler(toRad(-13.0), toRad(90.0), toRad(2.0))),
		0.01));

	horse->setLocalRotation(Mat3x4(0.135899, -0.534728, 0.834033, 0.000000,
		0.091205, 0.845038, 0.526900, 0.000000 ,
		-0.986537, 0.004463, 0.163603, 0.000000));
#endif
}

//==============================================================================
#if 0
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
#endif

//==============================================================================
I32 mainLoopExtra(App& app, void*)
{
	F32 dist = 0.1;
	F32 ang = toRad(2.5);
	F32 scale = 0.01;
	F32 mouseSensivity = 9.0;

	SceneGraph& scene = app.getSceneGraph();
	Input& in = app.getInput();
	MainRenderer& renderer = app.getMainRenderer();

	if(in.getKey(KeyCode::ESCAPE))
	{
		return 1;
	}

	// move the camera
	static MoveComponent* mover = 
		&scene.getActiveCamera().getComponent<MoveComponent>();

	if(in.getKey(KeyCode::_1))
	{
		mover = &scene.getActiveCamera();
	}
	if(in.getKey(KeyCode::_2))
	{
		mover = &scene.findSceneNode("horse").getComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_3))
	{
		mover = &scene.findSceneNode("spot0").getComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_4))
	{
		mover = &scene.findSceneNode("spot1").getComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_5))
	{
		mover = &scene.findSceneNode("pe").getComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_6))
	{
		mover = &scene.findSceneNode("shape0").getComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_7))
	{
		mover = &scene.findSceneNode("shape1").getComponent<MoveComponent>();
	}

	/*if(in.getKey(KeyCode::L) == 1)
	{
		SceneNode& l = 
			SceneGraphSingleton::get().findSceneNode("crate");
		
		Transform trf;
		trf.setIdentity();
		trf.getOrigin().y() = 20.0;
		l.getComponent<MoveComponent>().setLocalTransform(trf);
	}*/

	if(in.getKey(KeyCode::F1) == 1)
	{
		renderer.getDbg().setEnabled(!renderer.getDbg().getEnabled());
	}
	if(in.getKey(KeyCode::F2) == 1)
	{
		renderer.getDbg().switchBits(Dbg::Flag::SPATIAL);
	}
	if(in.getKey(KeyCode::F3) == 1)
	{
		renderer.getDbg().switchBits(Dbg::Flag::PHYSICS);
	}
	if(in.getKey(KeyCode::F4) == 1)
	{
		renderer.getDbg().switchBits(Dbg::Flag::SECTOR);
	}
	if(in.getKey(KeyCode::F5) == 1)
	{
		renderer.getDbg().switchBits(Dbg::Flag::OCTREE);
	}
	if(in.getKey(KeyCode::F6) == 1)
	{
		renderer.getDbg().switchDepthTestEnabled();
	}
	if(in.getKey(KeyCode::F12) == 1)
	{
		renderer.takeScreenshot("screenshot.tga");
	}

	if(in.getKey(KeyCode::UP)) mover->rotateLocalX(ang);
	if(in.getKey(KeyCode::DOWN)) mover->rotateLocalX(-ang);
	if(in.getKey(KeyCode::LEFT)) mover->rotateLocalY(ang);
	if(in.getKey(KeyCode::RIGHT)) mover->rotateLocalY(-ang);

	if(in.getKey(KeyCode::A))
	{
		mover->moveLocalX(-dist);
	}
	if(in.getKey(KeyCode::D)) mover->moveLocalX(dist);
	if(in.getKey(KeyCode::Z)) mover->moveLocalY(dist);
	if(in.getKey(KeyCode::SPACE)) mover->moveLocalY(-dist);
	if(in.getKey(KeyCode::W)) mover->moveLocalZ(-dist);
	if(in.getKey(KeyCode::S)) mover->moveLocalZ(dist);
	if(in.getKey(KeyCode::Q)) mover->rotateLocalZ(ang);
	if(in.getKey(KeyCode::E)) mover->rotateLocalZ(-ang);
	if(in.getKey(KeyCode::PAGEUP))
	{
		mover->scale(scale);
	}
	if(in.getKey(KeyCode::PAGEDOWN))
	{
		mover->scale(-scale);
	}
#if 0
	if(in.getKey(KeyCode::P) == 1)
	{
		std::cout << "{Vec3(" 
			<< mover->getWorldTransform().getOrigin().toString()
			<< "), Quat(" 
			<< Quat(mover->getWorldTransform().getRotation()).toString()
			<< ")}," << std::endl;
	}
#endif

#if 0
	if(in.getKey(KeyCode::L) == 1)
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
#endif

	if(in.getMousePosition() != Vec2(0.0))
	{
		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity *
			renderer.getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}

	//execStdinScpripts();

	if(getenv("PROFILE") && getGlobTimestamp() == 2000)
	{
		return 1;
	}

	return 0;
}

//==============================================================================
void initSubsystems(int argc, char* argv[])
{
	// Config
	Config config;
	config.set("ms.ez.enabled", false);
	config.set("ms.ez.maxObjectsToDraw", 100);
	config.set("dbg.enabled", false);
	config.set("is.sm.bilinearEnabled", true);
	config.set("is.groundLightEnabled", true);
	config.set("is.sm.enabled", true);
	config.set("is.sm.poissonEnabled", true);
	config.set("is.sm.resolution", 1024);
	config.set("pps.enabled", true);
	config.set("pps.hdr.enabled", true);
	config.set("pps.hdr.renderingQuality", 0.5);
	config.set("pps.hdr.blurringDist", 1.0);
	config.set("pps.hdr.blurringIterationsCount", 2);
	config.set("pps.hdr.exposure", 10.0);
	config.set("pps.hdr.samples", 17);
	config.set("pps.sslr.enabled", true);
	config.set("pps.sslr.renderingQuality", 0.5);
	config.set("pps.sslr.blurringIterationsCount", 1);
	config.set("pps.ssao.blurringIterationsCount", 2);
	config.set("pps.ssao.enabled", true);
	config.set("pps.ssao.renderingQuality", 0.35);
	config.set("pps.bl.enabled", true);
	config.set("pps.bl.blurringIterationsCount", 2);
	config.set("pps.bl.sideBlurFactor", 1.0);
	config.set("pps.lf.enabled", true);
	config.set("pps.sharpen", true);
	config.set("renderingQuality", 1.0);
	config.set("width", 1280);
	config.set("height", 720);
	config.set("lodDistance", 20.0);
	config.set("samples", 1);
	config.set("tessellation", false);
	config.set("tilesXCount", 16);
	config.set("tilesYCount", 16);

	config.set("fullscreenDesktopResolution", false);
	config.set("debugContext", false);

	app = new App(config, allocAligned, nullptr);

	// Input
	app->getInput().lockCursor(true);
	app->getInput().hideCursor(true);
	app->getInput().moveCursor(Vec2(0.0));
}

//==============================================================================
int main(int argc, char* argv[])
{
	int exitCode;

	try
	{
		initSubsystems(argc, argv);
		init();

		app->mainLoop(mainLoopExtra, nullptr);

		ANKI_LOGI("Exiting...");
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

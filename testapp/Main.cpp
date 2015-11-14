#include <stdio.h>
#include <iostream>
#include <fstream>

#ifdef ANKI_BUILD
#	undef ANKI_BUILD
#endif

#include "anki/input/Input.h"
#include "anki/Math.h"
#include "anki/Renderer.h"
#include "anki/core/App.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/script/ScriptManager.h"
#include "anki/core/StdinListener.h"
#include "anki/resource/Model.h"
#include "anki/resource/Script.h"
#include "anki/util/Logger.h"
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
#include "anki/event/JitterMoveEvent.h"
#include "anki/core/Config.h"
#include "anki/physics/PhysicsWorld.h"
#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/PlayerNode.h"
#include "anki/scene/PlayerControllerComponent.h"
#include "anki/Gr.h"

using namespace anki;

App* app;

ModelNode* horse;
PerspectiveCamera* cam;

#define PLAYER 0
#define MOUSE 1

Bool profile = false;

//==============================================================================
Error init()
{
	Error err = ErrorCode::NONE;
	ANKI_LOGI("Other init...");

	SpotLight* spot;
	PointLight* point;
	MoveComponent* move;
	LightComponent* lightc;

	SceneGraph& scene = app->getSceneGraph();
	MainRenderer& renderer = app->getMainRenderer();
	ResourceManager& resources = app->getResourceManager();

	scene.setAmbientColor(Vec4(1.0) * 0.05);
	renderer.getOffscreenRenderer().getPps().setFog(Vec3(1.0, 0.9, 0.9), 0.7);

	if(getenv("PROFILE"))
	{
		profile = true;
		app->setTimerTick(0.0);
	}

	// camera
	err = scene.newSceneNode<PerspectiveCamera>("main-camera", cam);
	if(err) return err;
	const F32 ang = 55.0;
	cam->setAll(
		renderer.getAspectRatio() * toRad(ang),
		toRad(ang), 0.2, 300.0);
	scene.setActiveCamera(cam);

	cam->getComponent<MoveComponent>().
		setLocalTransform(Transform(Vec4(0.0),
		Mat3x4(Euler(toRad(0.0), toRad(180.0), toRad(0.0))),
		1.0));

#if !PLAYER
	cam->getComponent<MoveComponent>().
		setLocalTransform(Transform(
		Vec4(147.392776, -10.132728, 16.607138, 0.0),
		//Vec4(0.0, 10, 0, 0),
		Mat3x4(Euler(toRad(0.0), toRad(90.0), toRad(0.0))),
		//Mat3x4::getIdentity(),
		1.0));
#endif

	if(0)
	{
		ReflectionProbe* refl;
		scene.newSceneNode<ReflectionProbe>("refl", refl, 68.0f);
		move = refl->tryGetComponent<MoveComponent>();
		move->setLocalOrigin(Vec4(0.0, 10, 0, 0));

		ReflectionProxy* proxy;
		scene.newSceneNode<ReflectionProxy>("proxy", proxy, 100.0, 15.0, 10.0);
		move = proxy->tryGetComponent<MoveComponent>();
		move->setLocalOrigin(Vec4(0.0, 12, -15, 0));
	}

#if 0
	PointLight* plight;
	scene.newSceneNode<PointLight>("spot0", plight);

	lightc = plight->tryGetComponent<LightComponent>();
	lightc->setDiffuseColor(Vec4(0.0, 30.0, 0.0, 1.0));
	lightc->setSpecularColor(Vec4(1.2));
	lightc->setDistance(5.0);

	move = plight->tryGetComponent<MoveComponent>();
	move->setLocalTransform(Transform(Vec4(0.0, 0.0, 0.0, 0.0),
		Mat3x4::getIdentity(), 1.0));
#endif
#if 0
	SpotLight* light;
	scene.newSceneNode<SpotLight>("spot0", light);

	lightc = light->tryGetComponent<LightComponent>();
	lightc->setOuterAngle(toRad(45.0));
	lightc->setInnerAngle(toRad(15.0));
	lightc->setDiffuseColor(Vec4(0.0, 30.0, 0.0, 1.0));
	lightc->setSpecularColor(Vec4(1.2));
	lightc->setDistance(5.0);

	move = light->tryGetComponent<MoveComponent>();
	move->setLocalTransform(Transform(Vec4(0.0, 0.5, 0.0, 0.0),
		Mat3x4::getIdentity(), 1.0));
#endif

#if 0
	err = scene.newSceneNode<SpotLight>("spot1", spot);
	if(err) return err;
	spot->setOuterAngle(toRad(45.0));
	spot->setInnerAngle(toRad(15.0));
	spot->setLocalTransform(Transform(Vec4(5.3, 4.3, 3.0, 0.0),
		Mat3x4::getIdentity(), 1.0));
	spot->setDiffuseColor(Vec4(3.0, 0.0, 0.0, 0.0));
	spot->setSpecularColor(Vec4(3.0, 3.0, 0.0, 0.0));
	spot->setDistance(30.0);
	spot->setShadowEnabled(true);
#endif

#if 0
	// Vase point lights
	F32 x = 8.5;
	F32 y = 2.25;
	F32 z = 2.49;
	Array<Vec3, 4> vaseLightPos = {{Vec3(x, y, -z - 1.4), Vec3(x, y, z),
		Vec3(-x - 2.3, y, z), Vec3(-x - 2.3, y, -z - 1.4)}};
	for(U i = 0; i < vaseLightPos.getSize(); i++)
	{
		Vec4 lightPos = vaseLightPos[i].xyz0();

		PointLight* point;
		err = scene.newSceneNode<PointLight>(
			("vase_plight" + std::to_string(i)).c_str(), point);
		if(err) return err;

		point->setRadius(2.0);
		point->setLocalOrigin(lightPos);
		point->setDiffuseColor(Vec4(3.0, 0.2, 0.0, 0.0));
		point->setSpecularColor(Vec4(1.0, 1.0, 0.0, 0.0));

		//if(i == 0)
		{
		point->loadLensFlare("textures/lens_flare/flares0.ankitex");
		LensFlareComponent& lf = point->getComponent<LensFlareComponent>();
		lf.setFirstFlareSize(Vec2(0.5, 0.2));
		lf.setColorMultiplier(Vec4(1.0, 1.0, 1.0, 0.6));
		}

		LightEvent* event;
		err = scene.getEventManager().newEvent(event, 0.0, 0.8, point);
		if(err) return err;
		event->setRadiusMultiplier(0.2);
		event->setIntensityMultiplier(Vec4(-1.2, 0.0, 0.0, 0.0));
		event->setSpecularIntensityMultiplier(Vec4(0.1, 0.1, 0.0, 0.0));
		event->setReanimate(true);

		JitterMoveEvent* mevent;
		scene.getEventManager().newEvent(mevent, 0.0, 2.0, point);
		mevent->setPositionLimits(
			Vec4(-0.5, 0.0, -0.5, 0), Vec4(0.5, 0.0, 0.5, 0));
		mevent->setReanimate(true);

		ParticleEmitter* pe;
		/**/

		if(i == 0)
		{
			err = scene.newSceneNode<ParticleEmitter>(
				"pefire", pe, "particles/fire.ankipart");
			if(err) return err;
			pe->setLocalOrigin(lightPos);

			err = scene.newSceneNode<ParticleEmitter>(
				"pesmoke", pe, "particles/smoke.ankipart");
			if(err) return err;
			pe->setLocalOrigin(lightPos);
		}
		else
		{
			InstanceNode* instance;
			err = scene.newSceneNode<InstanceNode>(
				("pefire_inst" + std::to_string(i)).c_str(), instance);
			if(err) return err;

			instance->setLocalOrigin(lightPos);

			SceneNode& sn = scene.findSceneNode("pefire");
			sn.addChild(instance);

			err = scene.newSceneNode<InstanceNode>(
				("pesmoke_inst" + std::to_string(i)).c_str(), instance);
			if(err) return err;

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

#if 0
	// horse
	err = scene.newSceneNode<ModelNode>("horse", horse,
		"models/horse/horse.ankimdl");
	if(err) return err;
	horse->getComponent<MoveComponent>().setLocalTransform(
		Transform(Vec4(0, 3, 0, 0.0), Mat3x4::getIdentity(), 0.5));
#endif

	if(0)
	{
		err = scene.newSceneNode<PointLight>("plight0", point);
		if(err) return err;

		lightc = point->tryGetComponent<LightComponent>();
		lightc->setDistance(2.2);
		lightc->setDiffuseColor(Vec4(1.0));
		lightc->setSpecularColor(Vec4(0.6, 0.6, 0.3, 1.0));

		move = point->tryGetComponent<MoveComponent>();
		move->setLocalOrigin(Vec4(3.0, 2.0, 0.2, 0.0));
		move->setLocalRotation(Mat3x4(Axisang(toRad(90.0), Vec3(0, 1, 0))));
	}

	{
		ScriptResourcePtr script;

		ANKI_CHECK(resources.loadResource("maps/hell/scene.lua", script));
		if(err) return err;

		err = app->getScriptManager().evalString(script->getSource());
		if(err) return err;
	}

	/*{
		SceneNode* node = scene.tryFindSceneNode("Point_026");
		LightEvent* event = scene.getEventManager().newEvent<LightEvent>(
			0.0f, 50.0f, node);
		event->setIntensityMultiplier(Vec4(1.5));
		event->setFrequency(3.0, 0.1);
	}*/

#if PLAYER
	PlayerNode* pnode;
	scene.newSceneNode<PlayerNode>("player", pnode,
		Vec4(147.392776, -11.132728, 16.607138, 0.0));

	pnode->addChild(cam);

#endif

#if 0
	{
		ModelNode* fog;
		scene.newSceneNode<ModelNode>("fog", fog,
			"models/fog/volumetric_fog_box.ankimdl");
		MoveComponent& move = fog->getComponent<MoveComponent>();
		//move.setLocalOrigin(Vec4(10.0, -19.0, 0.0, 0.0));
		move.setLocalOrigin(Vec4(10.0, -26.5, 0.0, 0.0));
		move.setLocalScale(100.0);
	}
#endif

	return ErrorCode::NONE;
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
Error mainLoopExtra(App& app, void*, Bool& quit)
{
	Error err = ErrorCode::NONE;
	F32 dist = 0.1;
	F32 ang = toRad(2.5);
	F32 scale = 0.01;
	F32 mouseSensivity = 9.0;
	quit = false;

	SceneGraph& scene = app.getSceneGraph();
	Input& in = app.getInput();
	MainRenderer& renderer = app.getMainRenderer();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return err;
	}

	// move the camera
	static MoveComponent* mover =
		&scene.getActiveCamera().getComponent<MoveComponent>();

	if(in.getKey(KeyCode::_1))
	{
		mover = scene.getActiveCamera().tryGetComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_2))
	{
		mover = &scene.findSceneNode("proxy").
			getComponent<MoveComponent>();
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

	if(in.getKey(KeyCode::L) == 1)
	{
		Vec3 origin = mover->getWorldTransform().getOrigin().xyz();
		printf("%f %f %f\n", origin.x(), origin.y(), origin.z());
	}

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
		//renderer.takeScreenshot("screenshot.tga");
	}

#if !PLAYER
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
#endif

#if !PLAYER && MOUSE
	if(in.getMousePosition() != Vec2(0.0) && !profile)
	{
		//printf("%f %f\n", in.getMousePosition().x(), in.getMousePosition().y());

		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity *
			renderer.getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}
#endif

	//execStdinScpripts();

	if(profile && app.getGlobalTimestamp() == 500)
	{
		quit = true;
		return err;
	}

	return err;
}

//==============================================================================
Error initSubsystems(int argc, char* argv[])
{
	Error err = ErrorCode::NONE;

	// Config
	Config config;
	config.set("ms.ez.enabled", false);
	config.set("ms.ez.maxObjectsToDraw", 100);
	config.set("dbg.enabled", false);
	config.set("is.sm.bilinearEnabled", true);
	config.set("is.groundLightEnabled", false);
	config.set("is.sm.enabled", true);
	config.set("is.sm.maxLights", 16);
	config.set("is.sm.poissonEnabled", true);
	config.set("is.sm.resolution", 1024);
	config.set("lf.maxFlares", 32);
	config.set("pps.enabled", false);
	config.set("pps.bloom.enabled", true);
	config.set("pps.bloom.renderingQuality", 0.5);
	config.set("pps.bloom.blurringDist", 1.0);
	config.set("pps.bloom.blurringIterationsCount", 3);
	config.set("pps.bloom.threshold", 2.0);
	config.set("pps.bloom.scale", 2.0);
	config.set("pps.bloom.samples", 17);
	config.set("pps.sslr.enabled", true);
	config.set("pps.sslr.renderingQuality", 0.25);
	config.set("pps.sslr.blurringIterationsCount", 0);
	config.set("pps.ssao.blurringIterationsCount", 2);
	config.set("pps.ssao.enabled", true);
	config.set("pps.ssao.renderingQuality", 0.35);
	config.set("pps.bl.enabled", true);
	config.set("pps.bl.blurringIterationsCount", 2);
	config.set("pps.bl.sideBlurFactor", 1.0);
	config.set("pps.sslf.enabled", true);
	config.set("pps.sharpen", true);
	config.set("renderingQuality", 1.0);
	config.set("width", 128);
	config.set("height", 128);
	config.set("lodDistance", 20.0);
	config.set("samples", 1);
	config.set("tessellation", true);
	//config.set("maxTextureSize", 256);
	config.set("ir.rendererSize", 64);
	config.set("fullscreenDesktopResolution", false);
	config.set("debugContext", false);
	if(getenv("ANKI_DATA_PATH"))
	{
		config.set("dataPaths", getenv("ANKI_DATA_PATH"));
	}
	else
	{
		config.set("dataPaths", "assets");
	}
	config.set("sceneFrameAllocatorSize", 1024 * 1024 * 10);
	//config.set("maxTextureSize", 256);
	//config.set("lodDistance", 3.0);

	app = new App;
	err = app->create(config, allocAligned, nullptr);
	if(err) return err;

	// Input
#if MOUSE
	app->getInput().lockCursor(true);
	app->getInput().hideCursor(true);
	app->getInput().moveCursor(Vec2(0.0));
#endif

	return err;
}

//==============================================================================
int main(int argc, char* argv[])
{
	Error err = ErrorCode::NONE;

	err = initSubsystems(argc, argv);

	if(!err)
	{
		err = init();
	}

	if(!err)
	{
		err = app->mainLoop(mainLoopExtra, nullptr);
	}

	if(err)
	{
		ANKI_LOGE("Error reported. See previous messages");
	}
	else
	{
		delete app;
		ANKI_LOGI("Bye!!");
	}

	return 0;
}

#include <stdio.h>
#include <iostream>
#include <fstream>

#ifdef ANKI_BUILD
#undef ANKI_BUILD
#endif

#include "anki/AnKi.h"

using namespace anki;

#define PLAYER 0
#define MOUSE 1

class MyApp : public App
{
public:
	Bool m_profile = false;

	Error init();
	Error userMainLoop(Bool& quit) override;
};

MyApp* app = nullptr;

//==============================================================================
Error MyApp::init()
{
	Error err = ErrorCode::NONE;
	ANKI_LOGI("Other init...");

	SceneGraph& scene = app->getSceneGraph();
	MainRenderer& renderer = app->getMainRenderer();
	ResourceManager& resources = app->getResourceManager();

	renderer.getOffscreenRenderer().getVolumetric().setFogParticleColor(
		Vec3(1.0, 0.9, 0.9), 0.7);
	if(getenv("PROFILE"))
	{
		m_profile = true;
		app->setTimerTick(0.0);
	}

	{
		ScriptResourcePtr script;

		// ANKI_CHECK(resources.loadResource("maps/hell/scene.lua", script));
		ANKI_CHECK(resources.loadResource("data/map/scene.lua", script));
		if(err)
			return err;

		err = app->getScriptManager().evalString(script->getSource());
		if(err)
			return err;

		app->getMainRenderer()
			.getOffscreenRenderer()
			.getPps();
			// .loadColorGradingTexture("textures/adis/dungeon.ankitex");
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error MyApp::userMainLoop(Bool& quit)
{
	Error err = ErrorCode::NONE;
	F32 dist = 0.1;
	F32 ang = toRad(2.5);
	F32 scale = 0.01;
	F32 mouseSensivity = 9.0;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Input& in = getInput();
	MainRenderer& renderer = getMainRenderer();

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
		mover = &scene.findSceneNode("proxy").getComponent<MoveComponent>();
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
		renderer.getDbg().flipFlags(DbgFlag::SPATIAL_COMPONENT);
	}
	if(in.getKey(KeyCode::F3) == 1)
	{
		renderer.getDbg().flipFlags(DbgFlag::PHYSICS);
	}
	if(in.getKey(KeyCode::F4) == 1)
	{
		renderer.getDbg().flipFlags(DbgFlag::SECTOR_COMPONENT);
	}
	if(in.getKey(KeyCode::F6) == 1)
	{
		renderer.getDbg().switchDepthTestEnabled();
	}
	if(in.getKey(KeyCode::F12) == 1)
	{
		printf("F12\n");
		scene.getActiveCamera()
			.getComponent<FrustumComponent>()
			.markShapeForUpdate();
		scene.getActiveCamera()
			.getComponent<FrustumComponent>()
			.markTransformForUpdate();
	}

#if !PLAYER
	if(in.getKey(KeyCode::UP))
		mover->rotateLocalX(ang);
	if(in.getKey(KeyCode::DOWN))
		mover->rotateLocalX(-ang);
	if(in.getKey(KeyCode::LEFT))
		mover->rotateLocalY(ang);
	if(in.getKey(KeyCode::RIGHT))
		mover->rotateLocalY(-ang);

	if(in.getKey(KeyCode::A))
	{
		mover->moveLocalX(-dist);
	}
	if(in.getKey(KeyCode::D))
		mover->moveLocalX(dist);
	if(in.getKey(KeyCode::Z))
		mover->moveLocalY(dist);
	if(in.getKey(KeyCode::SPACE))
		mover->moveLocalY(-dist);
	if(in.getKey(KeyCode::W))
		mover->moveLocalZ(-dist);
	if(in.getKey(KeyCode::S))
		mover->moveLocalZ(dist);
	if(in.getKey(KeyCode::Q))
		mover->rotateLocalZ(ang);
	if(in.getKey(KeyCode::E))
		mover->rotateLocalZ(-ang);
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
	if(in.getMousePosition() != Vec2(0.0) && !m_profile)
	{
		// printf("%f %f\n", in.getMousePosition().x(),
		// in.getMousePosition().y());

		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity
			* renderer.getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}
#endif

	if(m_profile && getGlobalTimestamp() == 500)
	{
		quit = true;
		return err;
	}

	return err;
}

//==============================================================================
Error init(int argc, char* argv[])
{
	Error err = ErrorCode::NONE;

	// Config
	Config config;
	config.set("dbg.enabled", false);
	config.set("sm.bilinearEnabled", true);
	config.set("is.groundLightEnabled", false);
	config.set("sm.enabled", true);
	config.set("sm.maxLights", 16);
	config.set("sm.poissonEnabled", false);
	config.set("sm.resolution", 512);
	config.set("lf.maxFlares", 32);
	config.set("pps.enabled", true);
	config.set("bloom.enabled", true);
	config.set("bloom.renderingQuality", 0.5);
	config.set("bloom.blurringDist", 1.0);
	config.set("bloom.blurringIterationsCount", 1);
	config.set("bloom.threshold", 2.0);
	config.set("bloom.scale", 2.0);
	config.set("bloom.samples", 17);
	config.set("ssao.blurringIterationsCount", 1);
	config.set("ssao.enabled", true);
	config.set("ssao.renderingQuality", 0.25);
	config.set("sslf.enabled", true);
	config.set("pps.sharpen", false);
	config.set("renderingQuality", 1.0);
	config.set("width", 1920);
	config.set("height", 1088);
	config.set("lodDistance", 20.0);
	config.set("samples", 1);
	config.set("tessellation", true);
	// config.set("maxTextureSize", 256);
	config.set("ir.enabled", true);
	// config.set("ir.clusterSizeZ", 32);
	config.set("sslr.enabled", false);
	config.set("ir.rendererSize", 64);
	config.set("fullscreenDesktopResolution", true);
	// config.set("clusterSizeZ", 16);
	config.set("debugContext", false);
	if(getenv("ANKI_DATA_PATH"))
	{
		config.set("dataPaths", getenv("ANKI_DATA_PATH"));
	}
	else
	{
		config.set("dataPaths", "assets");
	}
	// config.set("maxTextureSize", 256);
	// config.set("lodDistance", 3.0);

	// config.saveToFile("./config.xml");
	// config.loadFromFile("./config.xml");

	app = new MyApp;
	ANKI_CHECK(app->create(config, allocAligned, nullptr));
	ANKI_CHECK(app->init());

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

	err = init(argc, argv);

	if(!err)
	{
		err = app->mainLoop();
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

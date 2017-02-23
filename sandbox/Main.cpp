// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <cstdio>
#include <iostream>
#include <fstream>
#include <anki/AnKi.h>

using namespace anki;

#define PLAYER 0
#define MOUSE 1

class MyApp : public App
{
public:
	Bool m_profile = false;

	Error init(int argc, char* argv[]);
	Error userMainLoop(Bool& quit) override;
};

MyApp* app = nullptr;

Error MyApp::init(int argc, char* argv[])
{
	if(argc < 3)
	{
		ANKI_LOGE("usage: %s /path/to/config.xml relative/path/to/scene.lua", argv[0]);
		return ErrorCode::USER_DATA;
	}

	// Config
	Config config;
	ANKI_CHECK(config.loadFromFile(argv[1]));
	ANKI_CHECK(config.setFromCommandLineArguments(argc, argv));

	// Init super class
	ANKI_CHECK(App::init(config, allocAligned, nullptr));

	// Other init
	MainRenderer& renderer = getMainRenderer();
	ResourceManager& resources = getResourceManager();

	renderer.getOffscreenRenderer().getVolumetric().setFogParticleColor(Vec3(1.0, 0.9, 0.9) * 0.009);

	if(getenv("PROFILE"))
	{
		m_profile = true;
		setTimerTick(0.0);
	}

// Input
#if MOUSE
	getInput().lockCursor(true);
	getInput().hideCursor(true);
	getInput().moveCursor(Vec2(0.0));
#endif

	// Load scene
	ScriptResourcePtr script;
	ANKI_CHECK(resources.loadResource(argv[2], script));
	ANKI_CHECK(getScriptManager().evalString(script->getSource()));

	ANKI_CHECK(renderer.getOffscreenRenderer().getPps().loadColorGradingTexture(
		"textures/color_gradient_luts/forge_lut.ankitex"));

#if PLAYER
	SceneGraph& scene = getSceneGraph();
	SceneNode& cam = scene.getActiveCamera();

	PlayerNode* pnode;
	ANKI_CHECK(scene.newSceneNode<PlayerNode>(
		"player", pnode, cam.getComponent<MoveComponent>().getLocalOrigin() - Vec4(0.0, 1.0, 0.0, 0.0)));

	cam.getComponent<MoveComponent>().setLocalTransform(
		Transform(Vec4(0.0, 0.0, 0.0, 0.0), Mat3x4::getIdentity(), 1.0));

	pnode->addChild(&cam);
#endif

	return ErrorCode::NONE;
}

Error MyApp::userMainLoop(Bool& quit)
{
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
		return ErrorCode::NONE;
	}

	// move the camera
	static MoveComponent* mover = &scene.getActiveCamera().getComponent<MoveComponent>();

	if(in.getKey(KeyCode::_1))
	{
		mover = scene.getActiveCamera().tryGetComponent<MoveComponent>();
	}
	if(in.getKey(KeyCode::_2))
	{
		mover = &scene.findSceneNode("Lamp").getComponent<MoveComponent>();
	}

	if(in.getKey(KeyCode::L) == 1)
	{
		/*Vec3 origin = mover->getWorldTransform().getOrigin().xyz();
		printf("%f %f %f\n", origin.x(), origin.y(), origin.z());*/
		mover->setLocalOrigin(Vec4(81.169312, -2.309618, 17.088392, 0.0));
		// mover->setLocalRotation(Mat3x4::getIdentity());
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
		F32 angY = -ang * in.getMousePosition().x() * mouseSensivity * renderer.getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ang * in.getMousePosition().y() * mouseSensivity);
	}
#endif

	if(in.getEvent(InputEvent::WINDOW_CLOSED))
	{
		quit = true;
	}

	if(m_profile && getGlobalTimestamp() == 500)
	{
		quit = true;
		return ErrorCode::NONE;
	}

	return ErrorCode::NONE;
}

int main(int argc, char* argv[])
{
	Error err = ErrorCode::NONE;

	app = new MyApp;
	err = app->init(argc, argv);
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

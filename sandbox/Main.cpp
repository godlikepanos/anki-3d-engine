// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	Error userMainLoop(Bool& quit, Second elapsedTime) override;
};

MyApp* app = nullptr;

Error MyApp::init(int argc, char* argv[])
{
	if(argc < 2)
	{
		ANKI_LOGE("usage: %s relative/path/to/scene.lua [anki config options]", argv[0]);
		return Error::USER_DATA;
	}

	// Config
	ConfigSet config = DefaultConfigSet::get();
	ANKI_CHECK(config.setFromCommandLineArguments(argc - 2, argv + 2));

	// Init super class
	ANKI_CHECK(App::init(config, allocAligned, nullptr));

	// Other init
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();
	ResourceManager& resources = getResourceManager();

	renderer.getVolumetricFog().setFogParticleColor(Vec3(1.0f, 0.9f, 0.9f));
	renderer.getVolumetricFog().setParticleDensity(1.0f);

	if(getenv("PROFILE"))
	{
		m_profile = true;
		setTimerTick(0.0);
		TracerSingleton::get().setEnabled(true);
	}

// Input
#if MOUSE
	getInput().lockCursor(true);
	getInput().hideCursor(true);
	getInput().moveCursor(Vec2(0.0));
#endif

	// Load scene
	ScriptResourcePtr script;
	ANKI_CHECK(resources.loadResource(argv[1], script));
	ANKI_CHECK(getScriptManager().evalString(script->getSource()));

	// ANKI_CHECK(renderer.getFinalComposite().loadColorGradingTexture(
	//	"textures/color_gradient_luts/forge_lut.ankitex"));

#if PLAYER
	SceneGraph& scene = getSceneGraph();
	SceneNode& cam = scene.getActiveCameraNode();

	PlayerNode* pnode;
	ANKI_CHECK(scene.newSceneNode<PlayerNode>(
		"player", pnode, cam.getFirstComponentOfType<MoveComponent>().getLocalOrigin() - Vec4(0.0, 1.0, 0.0, 0.0)));

	cam.getFirstComponentOfType<MoveComponent>().setLocalTransform(
		Transform(Vec4(0.0, 0.0, 0.0, 0.0), Mat3x4::getIdentity(), 1.0));

	pnode->addChild(&cam);
#endif

	return Error::NONE;
}

Error MyApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	F32 ang = toRad(2.5f);
	F32 scale = 0.01f;
	F32 mouseSensivity = 9.0f;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Input& in = getInput();
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return Error::NONE;
	}

	// move the camera
	static MoveComponent* mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();

	if(in.getKey(KeyCode::_1))
	{
		mover = scene.getActiveCameraNode().tryGetFirstComponentOfType<MoveComponent>();
	}
	if(in.getKey(KeyCode::_2))
	{
		mover = &scene.findSceneNode("Point.018_Orientation").getFirstComponentOfType<MoveComponent>();
	}

	if(in.getKey(KeyCode::L) == 1)
	{
		/*Vec3 origin = mover->getWorldTransform().getOrigin().xyz();
		printf("%f %f %f\n", origin.x(), origin.y(), origin.z());*/
		mover->setLocalOrigin(Vec4(81.169312f, -2.309618f, 17.088392f, 0.0f));
		// mover->setLocalRotation(Mat3x4::getIdentity());
	}

	if(in.getKey(KeyCode::F1) == 1)
	{
		static U mode = 0;
		mode = (mode + 1) % 3;
		if(mode == 0)
		{
			renderer.getDbg().setEnabled(false);
		}
		else if(mode == 1)
		{
			renderer.getDbg().setEnabled(true);
			renderer.getDbg().setDepthTestEnabled(true);
			renderer.getDbg().setDitheredDepthTestEnabled(false);
		}
		else
		{
			renderer.getDbg().setEnabled(true);
			renderer.getDbg().setDepthTestEnabled(false);
			renderer.getDbg().setDitheredDepthTestEnabled(true);
		}
	}
	if(in.getKey(KeyCode::F2) == 1)
	{
		// renderer.getDbg().flipFlags(DbgFlag::SPATIAL_COMPONENT);
	}
	if(in.getKey(KeyCode::F3) == 1)
	{
		// renderer.getDbg().flipFlags(DbgFlag::PHYSICS);
	}
	if(in.getKey(KeyCode::F4) == 1)
	{
		// renderer.getDbg().flipFlags(DbgFlag::SECTOR_COMPONENT);
	}
	if(in.getKey(KeyCode::F6) == 1)
	{
		renderer.getDbg().switchDepthTestEnabled();
	}

	if(in.getKey(KeyCode::F12) == 1)
	{
		TracerSingleton::get().setEnabled(!TracerSingleton::get().getEnabled());
	}

#if !PLAYER
	static F32 dist = 0.1f;
	if(in.getMouseButton(MouseButton::SCROLL_UP) == 1)
	{
		dist += 0.1f;
		dist = min(dist, 10.0f);
	}
	if(in.getMouseButton(MouseButton::SCROLL_DOWN) == 1)
	{
		dist -= 0.1f;
		dist = max(dist, 0.1f);
	}

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
	if(in.getKey(KeyCode::SPACE))
		mover->moveLocalY(dist);
	if(in.getKey(KeyCode::C))
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

	if(in.getKey(KeyCode::U) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "SSGI") ? "" : "SSGI");
	}

	if(in.getKey(KeyCode::I) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "SSR") ? "" : "SSR");
	}

	if(in.getKey(KeyCode::O) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "SM_resolve") ? ""
																									  : "SM_resolve");
	}

	if(in.getKey(KeyCode::H) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "RtShadows") ? ""
																									 : "RtShadows");
	}

	if(in.getEvent(InputEvent::WINDOW_CLOSED))
	{
		quit = true;
	}

	if(m_profile && getGlobalTimestamp() == 1000)
	{
		quit = true;
		return Error::NONE;
	}

	return Error::NONE;
}

int main(int argc, char* argv[])
{
	Error err = Error::NONE;

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

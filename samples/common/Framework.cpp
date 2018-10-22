// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Framework.h"

using namespace anki;

Error SampleApp::init(int argc, char** argv, CString sampleName)
{
	if(!directoryExists("assets"))
	{
		ANKI_LOGE("Cannot find directory \"assets\". YOU ARE RUNNING THE SAMPLE FROM THE WRONG DIRECTORY. "
				  "To run %s you have to navigate to the /path/to/anki/samples/%s and then execute it",
			argv[0],
			&sampleName[0]);
		return Error::USER_DATA;
	}

	// Init the super class
	Config config;
	config.set("window.fullscreen", true);
	config.set("rsrc.dataPaths", ".:../..");
	config.set("window.debugContext", 0);
	ANKI_CHECK(config.setFromCommandLineArguments(argc, argv));
	ANKI_CHECK(App::init(config, allocAligned, nullptr));

	// Input
	getInput().lockCursor(true);
	getInput().hideCursor(true);
	getInput().moveCursor(Vec2(0.0f));

	// Some renderer stuff
	getMainRenderer().getOffscreenRenderer().getVolumetricFog().setFogParticleColor(Vec3(1.0f, 0.9f, 0.9f));

	ANKI_CHECK(sampleExtraInit());

	return Error::NONE;
}

Error SampleApp::userMainLoop(Bool& quit)
{
	const F32 MOVE_DISTANCE = 0.1f;
	const F32 ROTATE_ANGLE = toRad(2.5f);
	const F32 MOUSE_SENSITIVITY = 9.0f;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();
	Input& in = getInput();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return Error::NONE;
	}

	// move the camera
	static MoveComponent* mover = &scene.getActiveCameraNode().getComponent<MoveComponent>();

	if(in.getKey(KeyCode::_1) == 1)
	{
		mover = &scene.getActiveCameraNode().getComponent<MoveComponent>();
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

	if(in.getKey(KeyCode::UP))
	{
		mover->rotateLocalX(ROTATE_ANGLE);
	}

	if(in.getKey(KeyCode::DOWN))
	{
		mover->rotateLocalX(-ROTATE_ANGLE);
	}

	if(in.getKey(KeyCode::LEFT))
	{
		mover->rotateLocalY(ROTATE_ANGLE);
	}

	if(in.getKey(KeyCode::RIGHT))
	{
		mover->rotateLocalY(-ROTATE_ANGLE);
	}

	if(in.getKey(KeyCode::A))
	{
		mover->moveLocalX(-MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::D))
	{
		mover->moveLocalX(MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::C))
	{
		mover->moveLocalY(-MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::SPACE))
	{
		mover->moveLocalY(MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::W))
	{
		mover->moveLocalZ(-MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::S))
	{
		mover->moveLocalZ(MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::Q))
	{
		mover->rotateLocalZ(ROTATE_ANGLE);
	}

	if(in.getKey(KeyCode::E))
	{
		mover->rotateLocalZ(-ROTATE_ANGLE);
	}

	if(in.getKey(KeyCode::F12) == 1)
	{
		CoreTracerSingleton::get().m_enabled = !CoreTracerSingleton::get().m_enabled;
	}

	if(in.getMousePosition() != Vec2(0.0))
	{
		F32 angY = -ROTATE_ANGLE * in.getMousePosition().x() * MOUSE_SENSITIVITY * getMainRenderer().getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ROTATE_ANGLE * in.getMousePosition().y() * MOUSE_SENSITIVITY);
	}

	return Error::NONE;
}

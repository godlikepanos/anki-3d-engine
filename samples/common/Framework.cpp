// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Framework.h"

using namespace anki;

Error SampleApp::init(int argc, char** argv)
{
	// Init the super class
	Config config;
	config.set("fullscreenDesktopResolution", true);
	config.set("dataPaths", ".:../..");
	config.set("debugContext", 0);
	ANKI_CHECK(config.setFromCommandLineArguments(argc, argv));
	ANKI_CHECK(App::init(config, allocAligned, nullptr));

	// Input
	getInput().lockCursor(true);
	getInput().hideCursor(true);
	getInput().moveCursor(Vec2(0.0));

	// Some renderer stuff
	getMainRenderer().getOffscreenRenderer().getVolumetric().setFogParticleColor(Vec3(1.0, 0.9, 0.9) * 0.0001);

	ANKI_CHECK(sampleExtraInit());

	return ErrorCode::NONE;
}

Error SampleApp::userMainLoop(Bool& quit)
{
	const F32 MOVE_DISTANCE = 0.1;
	const F32 ROTATE_ANGLE = toRad(2.5);
	const F32 MOUSE_SENSITIVITY = 9.0;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();
	Input& in = getInput();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return ErrorCode::NONE;
	}

	// move the camera
	static MoveComponent* mover = &scene.getActiveCamera().getComponent<MoveComponent>();

	if(in.getKey(KeyCode::_1) == 1)
	{
		mover = &scene.getActiveCamera().getComponent<MoveComponent>();
	}

	if(in.getKey(KeyCode::F1) == 1)
	{
		renderer.getDbg().setEnabled(!renderer.getDbg().getEnabled());
	}
	if(in.getKey(KeyCode::F2) == 1)
	{
		renderer.getDbg().flipFlags(DbgFlag::SPATIAL_COMPONENT);
	}
	if(in.getKey(KeyCode::F6) == 1)
	{
		renderer.getDbg().switchDepthTestEnabled();
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

	if(in.getKey(KeyCode::Z))
	{
		mover->moveLocalY(MOVE_DISTANCE);
	}

	if(in.getKey(KeyCode::SPACE))
	{
		mover->moveLocalY(-MOVE_DISTANCE);
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

	if(in.getMousePosition() != Vec2(0.0))
	{
		F32 angY = -ROTATE_ANGLE * in.getMousePosition().x() * MOUSE_SENSITIVITY * getMainRenderer().getAspectRatio();

		mover->rotateLocalY(angY);
		mover->rotateLocalX(ROTATE_ANGLE * in.getMousePosition().y() * MOUSE_SENSITIVITY);
	}

	return ErrorCode::NONE;
}
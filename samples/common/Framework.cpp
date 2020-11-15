// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Framework.h"

using namespace anki;

Error SampleApp::init(int argc, char** argv, CString sampleName)
{
	HeapAllocator<U32> alloc(allocAligned, nullptr);
	StringAuto mainDataPath(alloc, ANKI_SOURCE_DIRECTORY);
	StringAuto assetsDataPath(alloc);
	assetsDataPath.sprintf("%s/samples/%s", ANKI_SOURCE_DIRECTORY, sampleName.cstr());

	if(!directoryExists(assetsDataPath))
	{
		ANKI_LOGE("Cannot find directory \"%s\". Have you moved the clone of the repository?", assetsDataPath.cstr());
		return Error::USER_DATA;
	}

	// Init the super class
	ConfigSet config = DefaultConfigSet::get();
	config.set("window_fullscreen", true);
	config.set("rsrc_dataPaths", StringAuto(alloc).sprintf("%s:%s", mainDataPath.cstr(), assetsDataPath.cstr()));
	config.set("gr_debugContext", 0);
	ANKI_CHECK(config.setFromCommandLineArguments(argc - 1, argv + 1));
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

Error SampleApp::userMainLoop(Bool& quit, Second elapsedTime)
{
	const F32 ROTATE_ANGLE = toRad(2.5f);
	const F32 MOUSE_SENSITIVITY = 5.0f;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Renderer& renderer = getMainRenderer().getOffscreenRenderer();
	Input& in = getInput();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return Error::NONE;
	}

	if(in.getKey(KeyCode::BACKQUOTE) == 1)
	{
		setDisplayDeveloperConsole(!getDisplayDeveloperConsole());
	}

	if(in.getKey(KeyCode::Y) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "GBuffer_velocity") ? "" : "GBuffer_velocity");
	}

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

	if(in.getKey(KeyCode::P) == 1)
	{
		renderer.setCurrentDebugRenderTarget(
			(renderer.getCurrentDebugRenderTarget() == "GBuffer_normals") ? "" : "GBuffer_normals");
	}

	if(in.getKey(KeyCode::H) == 1)
	{
		renderer.setCurrentDebugRenderTarget((renderer.getCurrentDebugRenderTarget() == "RtShadows") ? ""
																									 : "RtShadows");
	}

	if(!getDisplayDeveloperConsole())
	{
		in.hideCursor(true);
		in.lockCursor(true);

		// move the camera
		static MoveComponent* mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();

		if(in.getKey(KeyCode::_1) == 1)
		{
			mover = &scene.getActiveCameraNode().getFirstComponentOfType<MoveComponent>();
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

		static F32 moveDistance = 0.1f;
		if(in.getMouseButton(MouseButton::SCROLL_UP) == 1)
		{
			moveDistance += 0.1f;
			moveDistance = min(moveDistance, 10.0f);
		}

		if(in.getMouseButton(MouseButton::SCROLL_DOWN) == 1)
		{
			moveDistance -= 0.1f;
			moveDistance = max(moveDistance, 0.1f);
		}

		if(in.getKey(KeyCode::A))
		{
			mover->moveLocalX(-moveDistance);
		}

		if(in.getKey(KeyCode::D))
		{
			mover->moveLocalX(moveDistance);
		}

		if(in.getKey(KeyCode::C))
		{
			mover->moveLocalY(-moveDistance);
		}

		if(in.getKey(KeyCode::SPACE))
		{
			mover->moveLocalY(moveDistance);
		}

		if(in.getKey(KeyCode::W))
		{
			mover->moveLocalZ(-moveDistance);
		}

		if(in.getKey(KeyCode::S))
		{
			mover->moveLocalZ(moveDistance);
		}

		if(in.getKey(KeyCode::Q))
		{
			mover->rotateLocalZ(ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::E))
		{
			mover->rotateLocalZ(-ROTATE_ANGLE);
		}

		if(in.getKey(KeyCode::F12) == 1 && ANKI_ENABLE_TRACE)
		{
			TracerSingleton::get().setEnabled(!TracerSingleton::get().getEnabled());
		}

		if(in.getMousePosition() != Vec2(0.0))
		{
			Vec2 velocity = in.getMousePosition();
			Euler angles(mover->getLocalRotation().getRotationPart());
			angles.x() += velocity.y() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.x() = clamp(angles.x(), toRad(-90.0f), toRad(90.0f)); // Avoid cycle in Y axis
			angles.y() += -velocity.x() * toRad(360.0f) * F32(elapsedTime) * MOUSE_SENSITIVITY;
			angles.z() = 0.0f;
			mover->setLocalRotation(Mat3x4(Vec3(0.0f), angles));
		}
	}
	else
	{
		in.hideCursor(false);
		in.lockCursor(false);
	}

	return Error::NONE;
}

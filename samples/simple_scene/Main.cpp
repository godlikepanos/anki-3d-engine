#include <cstdio>
#include <anki/AnKi.h>

using namespace anki;

class MyApp : public App
{
public:
	Error init();
	Error userMainLoop(Bool& quit) override;
};

MyApp* app;

Error MyApp::init()
{
	// Init the super class
	Config config;
	config.set("fullscreenDesktopResolution", true);
	config.set("dataPaths", ".:..");
	config.set("debugContext", 0);
	ANKI_CHECK(App::init(config, allocAligned, nullptr));

	// Load the scene.lua
	ScriptResourcePtr script;
	ANKI_CHECK(getResourceManager().loadResource("samples/assets/scene.lua", script));
	ANKI_CHECK(getScriptManager().evalString(script->getSource()));

	// Input
	getInput().lockCursor(true);
	getInput().hideCursor(true);
	getInput().moveCursor(Vec2(0.0));

	// Some renderer stuff
	getMainRenderer().getOffscreenRenderer().getVolumetric().setFog(Vec3(1.0, 0.9, 0.9), 0.7);

	return ErrorCode::NONE;
}

Error MyApp::userMainLoop(Bool& quit)
{
	const F32 MOVE_DISTANCE = 0.1;
	const F32 ROTATE_ANGLE = toRad(2.5);
	const F32 MOUSE_SENSITIVITY = 9.0;
	quit = false;

	SceneGraph& scene = getSceneGraph();
	Input& in = getInput();

	if(in.getKey(KeyCode::ESCAPE))
	{
		quit = true;
		return ErrorCode::NONE;
	}

	// move the camera
	static MoveComponent* mover = &scene.getActiveCamera().getComponent<MoveComponent>();

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

int main(int argc, char* argv[])
{
	Error err = ErrorCode::NONE;

	app = new MyApp;
	err = app->init();
	if(!err)
	{
		err = app->mainLoop();
	}

	if(err)
	{
		ANKI_LOGE("Error reported. To run %s you have to navigate to the /path/to/anki/samples. And then execute it",
			argv[0]);
	}
	else
	{
		delete app;
		ANKI_LOGI("Bye!!");
	}

	return 0;
}

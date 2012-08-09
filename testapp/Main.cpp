#include <stdio.h>
#include <iostream>
#include <fstream>

#include "anki/input/Input.h"
#include "anki/scene/Camera.h"
#include "anki/math/Math.h"
#include "anki/renderer/Renderer.h"
#include "anki/ui/UiPainter.h"
#include "anki/core/App.h"
#include "anki/resource/Mesh.h"
#include "anki/scene/Light.h"
#include "anki/resource/Material.h"
#include "anki/scene/Scene.h"
#include "anki/resource/SkelAnim.h"
#include "anki/misc/Parser.h"
#include "anki/scene/ParticleEmitterNode.h"
#include "anki/physics/Character.h"
#include "anki/renderer/Renderer.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/physics/Character.h"
#include "anki/physics/RigidBody.h"
#include "anki/script/ScriptManager.h"
#include "anki/core/StdinListener.h"
#include "anki/scene/ModelNode.h"
#include "anki/resource/Model.h"
#include "anki/core/Logger.h"
#include "anki/util/Filesystem.h"
#include "anki/util/HighRezTimer.h"
#include "anki/scene/SkinNode.h"
#include "anki/resource/Skin.h"
#include "anki/ui/UiFtFontLoader.h"
#include "anki/ui/UiFont.h"
#include "anki/event/EventManager.h"
#include "anki/event/SceneColorEvent.h"
#include "anki/event/MainRendererPpsHdrEvent.h"
#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/Material.h"
#include "anki/core/ParallelManager.h"
#include "anki/misc/Xml.h"

using namespace anki;

UiPainter* painter;
ModelNode* horse;
PerspectiveCamera* cam;

//==============================================================================
void init()
{
	ANKI_LOGI("Other init...");

	Scene& scene = SceneSingleton::get();

	painter = new UiPainter(Vec2(AppSingleton::get().getWindowWidth(),
		AppSingleton::get().getWindowHeight()));
	painter->setFont("engine-rsrc/ModernAntiqua.ttf", 25, 25);

	// camera
	cam = new PerspectiveCamera("main-camera", &scene,
		Movable::MF_NONE, nullptr);
	const float ang = 70.0;
	cam->setAll(
		MainRendererSingleton::get().getAspectRatio() * Math::toRad(ang),
		Math::toRad(ang), 0.5, 200.0);
	cam->setLocalTransform(Transform(Vec3(0.0, 3.0, 8.0), Mat3::getIdentity(),
		1.0));
	scene.setActiveCamera(cam);

	// lights

	SpotLight* spot = new SpotLight("spot0", &scene, Movable::MF_NONE, nullptr);
	spot->setFov(Math::toRad(45.0));
	spot->setLocalTransform(Transform(Vec3(1.3, 4.3, 3.0),
		Mat3(Euler(Math::toRad(-20), Math::toRad(20), 0.0)), 1.0));
	spot->setDiffuseColor(Vec4(4.0));
	spot->setSpecularColor(Vec4(1.0));
	spot->loadTexture("gfx/lights/flashlight.tga");
	spot->setDistance(20.0);
	spot->setShadowEnabled(true);

	PointLight* point = new PointLight("point0", &scene, Movable::MF_NONE,
		nullptr);
	point->setRadius(3.0);
	point->setDiffuseColor(Vec4(1.0, 0.0, 0.0, 0.0));
	point->setSpecularColor(Vec4(0.0, 0.0, 1.0, 0.0));

	PointLight* point1 = new PointLight("point1", &scene, Movable::MF_NONE,
		nullptr);
	point1->setRadius(3.0);
	point1->setDiffuseColor(Vec4(2.0, 2.0, 2.0, 0.0));
	point1->setSpecularColor(Vec4(3.0, 3.0, 0.0, 0.0));
	point1->setLocalTranslation(Vec3(-3.0, 2.0, 0.0));

	// horse
	horse = new ModelNode("meshes/horse/horse.mdl", "horse", &scene,
		Movable::MF_NONE, nullptr);
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(),
		1.0));

	// Sectors
	scene.sectors.push_back(Sector(Aabb(Vec3(-10.0), Vec3(10.0))));
}

//==============================================================================
void mainLoopExtra()
{
	InputSingleton::get().handleEvents();

	float dist = 0.2;
	float ang = Math::toRad(3.0);
	float scale = 0.01;

	// move the camera
	static Movable* mover = SceneSingleton::get().getActiveCamera().getMovable();
	Input& in = InputSingleton::get();

	SceneSingleton::get().setAmbientColor(Vec3(0.2));

	if(in.getKey(SDL_SCANCODE_1))
	{
		mover = &SceneSingleton::get().getActiveCamera();
	}
	if(in.getKey(SDL_SCANCODE_3))
	{
		mover = SceneSingleton::get().findSceneNode("spot0")->getMovable();
	}
	if(in.getKey(SDL_SCANCODE_4))
	{
		mover = SceneSingleton::get().findSceneNode("point0")->getMovable();
	}
	if(in.getKey(SDL_SCANCODE_5))
	{
		mover = SceneSingleton::get().findSceneNode("point1")->getMovable();
	}

	if(in.getKey(SDL_SCANCODE_L) == 1)
	{
		Light* l = SceneSingleton::get().findSceneNode("point1")->getLight();
		static_cast<PointLight*>(l)->setRadius(10.0);
	}

	if(in.getKey(SDL_SCANCODE_UP)) mover->rotateLocalX(ang);
	if(in.getKey(SDL_SCANCODE_DOWN)) mover->rotateLocalX(-ang);
	if(in.getKey(SDL_SCANCODE_LEFT)) mover->rotateLocalY(ang);
	if(in.getKey(SDL_SCANCODE_RIGHT)) mover->rotateLocalY(-ang);

	if(in.getKey(SDL_SCANCODE_A)) mover->moveLocalX(-dist);
	if(in.getKey(SDL_SCANCODE_D)) mover->moveLocalX(dist);
	if(in.getKey(SDL_SCANCODE_LSHIFT)) mover->moveLocalY(dist);
	if(in.getKey(SDL_SCANCODE_SPACE)) mover->moveLocalY(-dist);
	if(in.getKey(SDL_SCANCODE_W)) mover->moveLocalZ(-dist);
	if(in.getKey(SDL_SCANCODE_S)) mover->moveLocalZ(dist);
	if(in.getKey(SDL_SCANCODE_Q)) mover->rotateLocalZ(ang);
	if(in.getKey(SDL_SCANCODE_E)) mover->rotateLocalZ(-ang);
	if(in.getKey(SDL_SCANCODE_PAGEUP))
	{
		mover->scale(scale);
	}
	if(in.getKey(SDL_SCANCODE_PAGEDOWN))
	{
		mover->scale(-scale);
	}
}

//==============================================================================
void mainLoop()
{
	ANKI_LOGI("Entering main loop");

	HighRezTimer mainLoopTimer;
	mainLoopTimer.start();
	HighRezTimer::Scalar prevUpdateTime = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar crntTime = prevUpdateTime;

	const char* vert = R"(
in vec3 position;
uniform mat4 mvp;

void main() {
	gl_Position = mvp * vec4(position, 1.0);
})";

	const char* frag = R"(
out vec3 fColor;
void main() 
{
	fColor = vec3(0.0, 1.0, 0.0);
})";

	const char* trf[] = {nullptr};

	ShaderProgram sprog;
	sprog.create(vert, nullptr, nullptr, nullptr, frag, trf);

	Vec3 maxPos = Vec3(0.5 * 1.0);
	Vec3 minPos = Vec3(-0.5 * 1.0);

	std::array<Vec3, 8> points = {{
		Vec3(maxPos.x(), maxPos.y(), maxPos.z()),  // right top front
		Vec3(minPos.x(), maxPos.y(), maxPos.z()),  // left top front
		Vec3(minPos.x(), minPos.y(), maxPos.z()),  // left bottom front
		Vec3(maxPos.x(), minPos.y(), maxPos.z()),  // right bottom front
		Vec3(maxPos.x(), maxPos.y(), minPos.z()),  // right top back
		Vec3(minPos.x(), maxPos.y(), minPos.z()),  // left top back
		Vec3(minPos.x(), minPos.y(), minPos.z()),  // left bottom back
		Vec3(maxPos.x(), minPos.y(), minPos.z())   // right bottom back
	}};

	std::array<uint16_t, 24> indeces = {{0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6,
		7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7}};

	Vbo posvbo, idsvbo;

	posvbo.create(GL_ARRAY_BUFFER, sizeof(points), &points[0], GL_STATIC_DRAW);
	idsvbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeces), &indeces[0], GL_STATIC_DRAW);

	Vao vao;
	vao.create();
	vao.attachArrayBufferVbo(posvbo, 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	vao.attachElementArrayBufferVbo(idsvbo);

	while(1)
	{
		SceneSingleton::get().updateFrameStart();
		HighRezTimer timer;
		timer.start();

		prevUpdateTime = crntTime;
		crntTime = HighRezTimer::getCurrentTime();

		// Update
		//
		mainLoopExtra();
		SceneSingleton::get().update(prevUpdateTime, crntTime);
		EventManagerSingleton::get().updateAllEvents(prevUpdateTime, crntTime);
		MainRendererSingleton::get().render(SceneSingleton::get());

		Fbo::unbindAll();

		/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);*/

		sprog.bind();
		sprog.findUniformVariable("mvp").set(
			cam->getProjectionMatrix() * cam->getViewMatrix());
		//horse->model->getModelPatches()[0].getVao(PassLevelKey(0, 0)).bind();
		vao.bind();

		//int indeces = horse->model->getModelPatches()[0].getIndecesNumber(0);
		int indeces = 24;
		glDrawElements(GL_LINES,
			indeces,
			GL_UNSIGNED_SHORT, 0);

		if(InputSingleton::get().getKey(SDL_SCANCODE_ESCAPE))
		{
			break;
		}

		AppSingleton::get().swapBuffers();

		// Sleep
		//
#if 1
		timer.stop();
		if(timer.getElapsedTime() < AppSingleton::get().getTimerTick())
		{
			SDL_Delay((AppSingleton::get().getTimerTick()
				- timer.getElapsedTime()) * 1000.0);
		}
#else
		if(MainRendererSingleton::get().getFramesCount() == 10000)
		{
			break;
		}
#endif
		SceneSingleton::get().updateFrameEnd();
	}

	ANKI_LOGI("Exiting main loop (" << mainLoopTimer.getElapsedTime()
		<< " sec)");
}


//==============================================================================
// initSubsystems                                                              =
//==============================================================================
void initSubsystems(int argc, char* argv[])
{
	// App
	AppSingleton::get().init(argc, argv);

	// Main renderer
	RendererInitializer initializer;
	initializer.ms.ez.enabled = true;
	initializer.dbg.enabled = true;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = true;
	initializer.is.sm.resolution = 1024;
	initializer.is.sm.level0Distance = 3.0;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.25;
	initializer.pps.hdr.blurringDist = 1.0;
	initializer.pps.hdr.blurringIterationsNum = 2;
	initializer.pps.hdr.exposure = 4.0;
	initializer.pps.ssao.blurringIterationsNum = 4;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.renderingQuality = 0.3;
	initializer.pps.bl.enabled = true;
	initializer.pps.bl.blurringIterationsNum = 2;
	initializer.pps.bl.sideBlurFactor = 1.0;
	initializer.mainRendererQuality = 1.0;

	MainRendererSingleton::get().init(initializer);

	// Stdin listener
	StdinListenerSingleton::get().start();

	// Parallel jobs
	ParallelManagerSingleton::get().init(4);
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
		std::cerr << "Aborting: " << e.what() << std::endl;
		exitCode = 1;
	}
	ANKI_LOGI("Bye!!");
	return exitCode;
}

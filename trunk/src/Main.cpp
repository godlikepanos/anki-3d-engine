#include <stdio.h>
#include <iostream>
#include <fstream>

#include "Input.h"
#include "Camera.h"
#include "Math.h"
#include "Renderer.h"
#include "Ui.h"
#include "App.h"
#include "Mesh.h"
#include "Light.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"
#include "Scene.h"
#include "Scanner.h"
#include "skybox.h"
#include "map.h"
#include "SkelAnim.h"
#include "LightRsrc.h"
#include "Parser.h"
#include "ParticleEmitter.h"
#include "PhyCharacter.h"
#include "Renderer.h"
#include "RendererInitializer.h"
#include "MainRenderer.h"
#include "PhyCharacter.h"
#include "RigidBody.h"
#include "ScriptingEngine.h"
#include "StdinListener.h"
#include "ModelNode.h"
#include "SkelAnimModelNodeCtrl.h"
#include "Model.h"
#include "Logger.h"
#include "Util.h"
#include "HighRezTimer.h"
#include "SkinNode.h"
#include "Skin.h"


// map (hard coded)
ModelNode* floor__,* sarge,* horse,* crate;
SkinNode* imp;
//SkelModelNode* imp;
PointLight* point_lights[10];
SpotLight* spot_lights[2];
ParticleEmitter* partEmitter;
PhyCharacter* character;


// Physics
Vec<btRigidBody*> boxes;

#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5

#define MAX_PROXIES (ARRAY_SIZE_X*ARRAY_SIZE_Y*ARRAY_SIZE_Z + 1024)

#define SCALING 1.
#define START_POS_X -5
#define START_POS_Y -5
#define START_POS_Z -3


void initPhysics()
{
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.),btScalar(50.),btScalar(50.)));

	Transform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(Vec3(0,-50, 0));

	RigidBody::Initializer init;
	init.mass = 0.0;
	init.shape = groundShape;
	init.startTrf = groundTransform;

	new RigidBody(SceneSingleton::getInstance().getPhysics(), init);


	/*{
		btCollisionShape* colShape = new btBoxShape(btVector3(SCALING*1,SCALING*1,SCALING*1));

		float start_x = START_POS_X - ARRAY_SIZE_X/2;
		float start_y = START_POS_Y;
		float start_z = START_POS_Z - ARRAY_SIZE_Z/2;

		for (int k=0;k<ARRAY_SIZE_Y;k++)
		{
			for (int i=0;i<ARRAY_SIZE_X;i++)
			{
				for(int j = 0;j<ARRAY_SIZE_Z;j++)
				{
					//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
					MeshNode* crate = new MeshNode;
					crate->init("models/crate0/crate0.mesh");
					crate->getLocalTransform().setScale(1.11);

					Transform trf(SCALING*Vec3(2.0*i + start_x, 20+2.0*k + start_y, 2.0*j + start_z), Mat3::getIdentity(), 1.0);
					new RigidBody(1.0, trf, colShape, crate, Physics::CG_MAP, Physics::CG_ALL);
				}
			}
		}
	}*/
}



//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void init()
{
	INFO("Other init...");

	srand(unsigned(time(NULL)));

	uint ticks = AppSingleton::getInstance().getTicks();

	//Ui::init();

	// camera
	Camera* cam = new Camera(false, NULL);
	cam->setAll(MainRendererSingleton::getInstance().getAspectRatio()*toRad(60.0), toRad(60.0), 0.5, 200.0);
	cam->moveLocalY(3.0);
	cam->moveLocalZ(5.7);
	cam->moveLocalX(-0.3);
	AppSingleton::getInstance().setActiveCam(cam);

	// lights
	point_lights[0] = new PointLight();
	point_lights[0]->init("maps/temple/light0.light");
	point_lights[0]->setLocalTransform(Transform(Vec3(-1.0, 2.4, 1.0), Mat3::getIdentity(), 1.0));
	point_lights[1] = new PointLight();
	point_lights[1]->init("maps/temple/light1.light");
	point_lights[1]->setLocalTransform(Transform(Vec3(2.5, 1.4, 1.0), Mat3::getIdentity(), 1.0));

	spot_lights[0] = new SpotLight();
	spot_lights[0]->init("maps/temple/light2.light");
	spot_lights[0]->setLocalTransform(Transform(Vec3(1.3, 4.3, 3.0), Mat3(Euler(toRad(-20), toRad(20), 0.0)), 1.0));
	spot_lights[1] = new SpotLight();
	spot_lights[1]->init("maps/temple/light3.light");
	spot_lights[1]->setLocalTransform(Transform(Vec3(-2.3, 6.3, 2.9), Mat3(Euler(toRad(-70), toRad(-20), 0.0)), 1.0));


	// horse
	horse = new ModelNode();
	horse->init("meshes/horse/horse.mdl");
	horse->setLocalTransform(Transform(Vec3(-2, 0, 0), Mat3::getIdentity(), 1.0));

	// Sponza
	ModelNode* sponza = new ModelNode();
	//sponza->init("maps/sponza/sponza.mdl");
	sponza->init("maps/sponza-crytek/sponza_crytek.mdl");
	sponza->setLocalTransform(Transform(Vec3(0.0), Mat3::getIdentity(), 0.05));

	// Pentagram
	ModelNode* pentagram = new ModelNode();
	pentagram->init("models/pentagram/pentagram.mdl");
	pentagram->setLocalTransform(Transform(Vec3(2, 0, 0), Mat3::getIdentity(), 1.0));


	// Imp
	imp = new SkinNode();
	imp->init("models/imp/imp.skin");
	imp->skelAnimModelNodeCtrl = new SkelAnimModelNodeCtrl(*imp);
	imp->skelAnimModelNodeCtrl->set(imp->getSkin().getSkelAnims()[0].get());
	imp->skelAnimModelNodeCtrl->setStep(0.8);

	return;

	// sarge
	/*sarge = new MeshNode();
	sarge->init("meshes/sphere/sphere16.mesh");
	//sarge->setLocalTransform(Vec3(0, -2.8, 1.0), Mat3(Euler(-M::PI/2, 0.0, 0.0)), 1.1);
	sarge->setLocalTransform(Transform(Vec3(0, 2.0, 2.0), Mat3::getIdentity(), 0.4));
	
	// floor
	floor__ = new MeshNode();
	floor__->init("maps/temple/Cube.019.mesh");
	floor__->setLocalTransform(Transform(Vec3(0.0, -0.19, 0.0), Mat3(Euler(-M::PI/2, 0.0, 0.0)), 0.8));*/

	// imp	
	/*imp = new SkelModelNode();
	imp->init("models/imp/imp.smdl");
	//imp->setLocalTransform(Transform(Vec3(0.0, 2.11, 0.0), Mat3(Euler(-M::PI/2, 0.0, 0.0)), 0.7));
	SkelAnimCtrl* ctrl = new SkelAnimCtrl(*imp->meshNodes[0]->meshSkelCtrl->skelNode);
	ctrl->skelAnim.loadRsrc("models/imp/walk.imp.anim");
	ctrl->step = 0.8;*/

	// cave map
	/*for(int i=1; i<21; i++)
	{
		MeshNode* node = new MeshNode();
		node->init(("maps/cave/rock." + lexical_cast<string>(i) + ".mesh").c_str());
		node->setLocalTransform(Transform(Vec3(0.0, -0.0, 0.0), Mat3::getIdentity(), 0.01));
	}*/

	// sponza map
	/*MeshNode* node = new MeshNode();
	node->init("maps/sponza/floor.mesh");
	node = new MeshNode();
	node->init("maps/sponza/walls.mesh");
	node = new MeshNode();
	node->init("maps/sponza/light-marbles.mesh");
	node = new MeshNode();
	node->init("maps/sponza/dark-marbles.mesh");*/
	//node->setLocalTransform(Transform(Vec3(0.0, -0.0, 0.0), Mat3::getIdentity(), 0.01));

	return;

	// particle emitter
	partEmitter = new ParticleEmitter;
	partEmitter->init("asdf");
	partEmitter->getLocalTransform().setOrigin(Vec3(3.0, 0.0, 0.0));

	// character
	/*PhyCharacter::Initializer init;
	init.sceneNode = imp;
	init.startTrf = Transform(Vec3(0, 40, 0), Mat3::getIdentity(), 1.0);
	character = new PhyCharacter(SceneSingleton::getInstance().getPhysics(), init);*/

	// crate
	/*crate = new MeshNode;
	crate->init("models/crate0/crate0.mesh");
	crate->scaleLspace = 1.0;*/


	//
	//floor_ = new floor_t;
	//floor_->material = RsrcMngr::materials.load("materials/default.mtl");


	initPhysics();

	INFO("Engine initialization ends (" << (App::getTicks() - ticks) << ")");
}


//======================================================================================================================
//                                                                                                                     =
//======================================================================================================================
void mainLoopExtra()
{
	InputSingleton::getInstance().handleEvents();

	float dist = 0.2;
	float ang = toRad(3.0);
	float scale = 0.01;

	// move the camera
	static SceneNode* mover = AppSingleton::getInstance().getActiveCam();

	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_1)) mover = AppSingleton::getInstance().getActiveCam();
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_2)) mover = point_lights[0];
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_3)) mover = spot_lights[0];
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_4)) mover = point_lights[1];
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_5)) mover = spot_lights[1];
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_6)) mover = imp;
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_M) == 1) InputSingleton::getInstance().warpMouse = !InputSingleton::getInstance().warpMouse;

	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_A)) mover->moveLocalX(-dist);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_D)) mover->moveLocalX(dist);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_LSHIFT)) mover->moveLocalY(dist);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_SPACE)) mover->moveLocalY(-dist);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_W)) mover->moveLocalZ(-dist);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_S)) mover->moveLocalZ(dist);
	if(!InputSingleton::getInstance().warpMouse)
	{
		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_UP)) mover->rotateLocalX(ang);
		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_DOWN)) mover->rotateLocalX(-ang);
		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_LEFT)) mover->rotateLocalY(ang);
		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_RIGHT)) mover->rotateLocalY(-ang);
	}
	else
	{
		float accel = 44.0;
		mover->rotateLocalX(ang * InputSingleton::getInstance().mouseVelocity.y() * accel);
		mover->rotateLocalY(-ang * InputSingleton::getInstance().mouseVelocity.x() * accel);
	}
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_Q)) mover->rotateLocalZ(ang);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_E)) mover->rotateLocalZ(-ang);
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_PAGEUP)) mover->getLocalTransform().getScale() += scale ;
	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_PAGEDOWN)) mover->getLocalTransform().getScale() -= scale ;

	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_K)) AppSingleton::getInstance().getActiveCam()->lookAtPoint(point_lights[0]->getWorldTransform().getOrigin());

	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_I))
		character->moveForward(0.1);


	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_O) == 1)
	{
		btRigidBody* body = static_cast<btRigidBody*>(boxes[0]);
		//body->getMotionState()->setWorldTransform(toBt(Mat4(Vec3(0.0, 10.0, 0.0), Mat3::getIdentity(), 1.0)));
		body->setWorldTransform(toBt(Mat4(Vec3(0.0, 10.0, 0.0), Mat3::getIdentity(), 1.0)));
		//body->clearForces();
		body->forceActivationState(ACTIVE_TAG);
	}

	if(InputSingleton::getInstance().getKey(SDL_SCANCODE_Y) == 1)
	{
		INFO("Exec script");
		ScriptingEngineSingleton::getInstance().execScript(Util::readFile("test.py").c_str());
	}

	mover->getLocalTransform().getRotation().reorthogonalize();

	//INFO(mover->getSceneNodeName())

	if(spot_lights[0]->getCamera().insideFrustum(spot_lights[1]->getCamera()))
	{
		INFO("in");
	}
	else
	{
		INFO("out");
	}
}


//======================================================================================================================
// mainLoop                                                                                                            =
//======================================================================================================================
void mainLoop()
{
	INFO("Entering main loop");

	int ticks = App::getTicks();
	do
	{
		HighRezTimer timer;
		timer.start();

		mainLoopExtra();

		AppSingleton::getInstance().execStdinScpripts();
		SceneSingleton::getInstance().getPhysics().update(timer.getCrntTime());
		SceneSingleton::getInstance().updateAllWorldStuff();
		//SceneSingleton::getInstance().doVisibilityTests(*AppSingleton::getInstance().getActiveCam());
		SceneSingleton::getInstance().doVisibilityTests(spot_lights[0]->getCamera());
		AppSingleton::getInstance().getActiveCam()->getVisibleMsRenderableNodes().clear();
		AppSingleton::getInstance().getActiveCam()->getVisibleMsRenderableNodes() = spot_lights[0]->getCamera().getVisibleMsRenderableNodes();
		AppSingleton::getInstance().getActiveCam()->getVisiblePointLights() = spot_lights[0]->getCamera().getVisiblePointLights();
		AppSingleton::getInstance().getActiveCam()->getVisibleSpotLights() = spot_lights[0]->getCamera().getVisibleSpotLights();

		SceneSingleton::getInstance().updateAllControllers();

		MainRendererSingleton::getInstance().render(*AppSingleton::getInstance().getActiveCam());

		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_ESCAPE))
		{
			break;
		}

		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_F11))
		{
			AppSingleton::getInstance().togleFullScreen();
		}

		if(InputSingleton::getInstance().getKey(SDL_SCANCODE_F12) == 1)
		{
			MainRendererSingleton::getInstance().takeScreenshot("gfx/screenshot.jpg");
		}

		AppSingleton::getInstance().swapBuffers();

		uint timeToSpendForRsrcPostProcess = AppSingleton::getInstance().getTimerTick() - timer.getElapsedTime();
		ResourceManagerSingleton::getInstance().postProcessFinishedLoadingRequests(timeToSpendForRsrcPostProcess);

		if(1)
		{
			//AppSingleton::getInstance().waitForNextFrame();

			timer.stop();
			if(timer.getElapsedTime() < AppSingleton::getInstance().getTimerTick())
			{
				SDL_Delay(AppSingleton::getInstance().getTimerTick() - timer.getElapsedTime());
			}
		}
		else
		{
			if(MainRendererSingleton::getInstance().getFramesNum() == 5000)
			{
				break;
			}
		}
	}while(true);

	INFO("Exiting main loop (" << (App::getTicks() - ticks) << ")");
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
int main(int argc, char* argv[])
{
	try
	{
		AppSingleton::getInstance().init(argc, argv);
		init();

		mainLoop();

		INFO("Exiting...");
		AppSingleton::getInstance().quit(EXIT_SUCCESS);
		return 0;
	}
	catch(std::exception& e)
	{
		ERROR("Aborting: " << e.what());
		//abort();
		return 1;
	}
}

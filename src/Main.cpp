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
#include "Resource.h"
#include "Scene.h"
#include "Scanner.h"
#include "skybox.h"
#include "map.h"
#include "MeshNode.h"
#include "SkelModelNode.h"
#include "MeshNode.h"
#include "SkelAnim.h"
#include "MeshSkelNodeCtrl.h"
#include "SkelAnimCtrl.h"
#include "SkelNode.h"
#include "LightData.h"
#include "Parser.h"
#include "ParticleEmitter.h"
#include "PhyCharacter.h"
#include "Renderer.h"
#include "RendererInitializer.h"
#include "MainRenderer.h"
#include "DebugDrawer.h"
#include "PhyCharacter.h"
#include "RigidBody.h"
#include "ScriptingEngine.h"
#include "StdinListener.h"
#include "Messaging.h"

// map (hard coded)
MeshNode* floor__,* sarge,* horse,* crate;
SkelModelNode* imp;
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
	groundTransform.origin = Vec3(0,-50, 0);

	RigidBody::Initializer init;
	init.mass = 0.0;
	init.shape = groundShape;
	init.startTrf = groundTransform;

	new RigidBody(app->getScene().getPhysics(), init);


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
	INFO("Engine initializing...");

	srand(unsigned(time(NULL)));
	mathSanityChecks();

	app->initWindow();
	uint ticks = app->getTicks();

	RendererInitializer initializer;
	initializer.ms.ez.enabled = false;
	initializer.dbg.enabled = true;
	initializer.is.sm.bilinearEnabled = true;
	initializer.is.sm.enabled = true;
	initializer.is.sm.pcfEnabled = true;
	initializer.is.sm.resolution = 512;
	initializer.pps.hdr.enabled = true;
	initializer.pps.hdr.renderingQuality = 0.25;
	initializer.pps.hdr.blurringDist = 1.0;
	initializer.pps.hdr.blurringIterations = 2;
	initializer.pps.hdr.exposure = 4.0;
	initializer.pps.ssao.blurringIterations = 2;
	initializer.pps.ssao.enabled = true;
	initializer.pps.ssao.renderingQuality = 0.5;
	initializer.mainRendererQuality = 1.0;
	app->getMainRenderer().init(initializer);
	//Ui::init();

	// camera
	Camera* cam = new Camera(app->getMainRenderer().getAspectRatio()*toRad(60.0), toRad(60.0), 0.5, 200.0);
	cam->moveLocalY(3.0);
	cam->moveLocalZ(5.7);
	cam->moveLocalX(-0.3);
	app->setActiveCam(cam);

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

	/*const char* skybox_fnames [] = { "textures/env/hellsky4_forward.tga", "textures/env/hellsky4_back.tga", "textures/env/hellsky4_left.tga",
																	 "textures/env/hellsky4_right.tga", "textures/env/hellsky4_up.tga", "textures/env/hellsky4_down.tga" };
	app->getScene().skybox.load(skybox_fnames);*/

	// horse
	horse = new MeshNode();
	horse->init("meshes/horse/horse.mesh");
	//horse->init("models/head/head.mesh");
	horse->setLocalTransform(Transform(Vec3(-2, 0, 1), Mat3::getIdentity(), 1.0));

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
	imp = new SkelModelNode();
	imp->init("models/imp/imp.smdl");
	//imp->setLocalTransform(Transform(Vec3(0.0, 2.11, 0.0), Mat3(Euler(-M::PI/2, 0.0, 0.0)), 0.7));
	SkelAnimCtrl* ctrl = new SkelAnimCtrl(*imp->meshNodes[0]->meshSkelCtrl->skelNode);
	ctrl->skelAnim.loadRsrc("models/imp/walk.imp.anim");
	ctrl->step = 0.8;

	// cave map
	/*for(int i=1; i<21; i++)
	{
		MeshNode* node = new MeshNode();
		node->init(("maps/cave/rock." + lexical_cast<string>(i) + ".mesh").c_str());
		node->setLocalTransform(Transform(Vec3(0.0, -0.0, 0.0), Mat3::getIdentity(), 0.01));
	}*/

	// sponza map
	MeshNode* node = new MeshNode();
	node->init("maps/sponza/floor.mesh");
	node = new MeshNode();
	node->init("maps/sponza/walls.mesh");
	node = new MeshNode();
	node->init("maps/sponza/light-marbles.mesh");
	node = new MeshNode();
	node->init("maps/sponza/dark-marbles.mesh");
	//node->setLocalTransform(Transform(Vec3(0.0, -0.0, 0.0), Mat3::getIdentity(), 0.01));

	return;

	// particle emitter
	partEmitter = new ParticleEmitter;
	partEmitter->init("asdf");
	partEmitter->getLocalTransform().origin = Vec3(3.0, 0.0, 0.0);

	// character
	PhyCharacter::Initializer init;
	init.sceneNode = imp;
	init.startTrf = Transform(Vec3(0, 40, 0), Mat3::getIdentity(), 1.0);
	character = new PhyCharacter(app->getScene().getPhysics(), init);

	// crate
	/*crate = new MeshNode;
	crate->init("models/crate0/crate0.mesh");
	crate->scaleLspace = 1.0;*/


	//
	//floor_ = new floor_t;
	//floor_->material = RsrcMngr::materials.load("materials/default.mtl");


	initPhysics();

	INFO("Engine initialization ends (" + boost::lexical_cast<std::string>(App::getTicks() - ticks) + ")");
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
		float crntTime = App::getTicks() / 1000.0;
		app->getInput().handleEvents();

		float dist = 0.2;
		float ang = toRad(3.0);
		float scale = 0.01;

		// move the camera
		static SceneNode* mover = app->getActiveCam();

		if(app->getInput().keys[SDL_SCANCODE_1]) mover = app->getActiveCam();
		if(app->getInput().keys[SDL_SCANCODE_2]) mover = point_lights[0];
		if(app->getInput().keys[SDL_SCANCODE_3]) mover = spot_lights[0];
		if(app->getInput().keys[SDL_SCANCODE_4]) mover = point_lights[1];
		if(app->getInput().keys[SDL_SCANCODE_5]) mover = spot_lights[1];
		if(app->getInput().keys[SDL_SCANCODE_6]) mover = partEmitter;
		if(app->getInput().keys[SDL_SCANCODE_M] == 1) app->getInput().warpMouse = !app->getInput().warpMouse;

		if(app->getInput().keys[SDL_SCANCODE_A]) mover->moveLocalX(-dist);
		if(app->getInput().keys[SDL_SCANCODE_D]) mover->moveLocalX(dist);
		if(app->getInput().keys[SDL_SCANCODE_LSHIFT]) mover->moveLocalY(dist);
		if(app->getInput().keys[SDL_SCANCODE_SPACE]) mover->moveLocalY(-dist);
		if(app->getInput().keys[SDL_SCANCODE_W]) mover->moveLocalZ(-dist);
		if(app->getInput().keys[SDL_SCANCODE_S]) mover->moveLocalZ(dist);
		if(!app->getInput().warpMouse)
		{
			if(app->getInput().keys[SDL_SCANCODE_UP]) mover->rotateLocalX(ang);
			if(app->getInput().keys[SDL_SCANCODE_DOWN]) mover->rotateLocalX(-ang);
			if(app->getInput().keys[SDL_SCANCODE_LEFT]) mover->rotateLocalY(ang);
			if(app->getInput().keys[SDL_SCANCODE_RIGHT]) mover->rotateLocalY(-ang);
		}
		else
		{
			float accel = 44.0;
			mover->rotateLocalX(ang * app->getInput().mouseVelocity.y * accel);
			mover->rotateLocalY(-ang * app->getInput().mouseVelocity.x * accel);
		}
		if(app->getInput().keys[SDL_SCANCODE_Q]) mover->rotateLocalZ(ang);
		if(app->getInput().keys[SDL_SCANCODE_E]) mover->rotateLocalZ(-ang);
		if(app->getInput().keys[SDL_SCANCODE_PAGEUP]) mover->getLocalTransform().scale += scale ;
		if(app->getInput().keys[SDL_SCANCODE_PAGEDOWN]) mover->getLocalTransform().scale -= scale ;

		if(app->getInput().keys[SDL_SCANCODE_K]) app->getActiveCam()->lookAtPoint(point_lights[0]->getWorldTransform().origin);

		if(app->getInput().keys[SDL_SCANCODE_I])
			character->moveForward(0.1);


		if(app->getInput().keys[SDL_SCANCODE_O] == 1)
		{
			btRigidBody* body = static_cast<btRigidBody*>(boxes[0]);
			//body->getMotionState()->setWorldTransform(toBt(Mat4(Vec3(0.0, 10.0, 0.0), Mat3::getIdentity(), 1.0)));
			body->setWorldTransform(toBt(Mat4(Vec3(0.0, 10.0, 0.0), Mat3::getIdentity(), 1.0)));
			//body->clearForces();
			body->forceActivationState(ACTIVE_TAG);
		}

		if(app->getInput().keys[SDL_SCANCODE_Y] == 1)
		{
			INFO("Exec script");
			app->getScriptingEngine().exposeVar("app", app);
			app->getScriptingEngine().execScript(Util::readFile("test.py").c_str());
		}

		mover->getLocalTransform().rotation.reorthogonalize();

		app->execStdinScpripts();
		app->getScene().getPhysics().update(crntTime);
		app->getScene().updateAllControllers();
		app->getScene().updateAllWorldStuff();

		app->getMainRenderer().render(*app->getActiveCam());

		//map.octree.root->bounding_box.render();

		// print some debug stuff
		/*Ui::setColor(Vec4(1.0, 1.0, 1.0, 1.0));
		Ui::setPos(-0.98, 0.95);
		Ui::setFontWidth(0.03);*/
		//Ui::printf("frame:%d fps:%dms\n", app->getMainRenderer().getFramesNum(), (App::getTicks()-ticks_));
		//Ui::print("Movement keys: arrows,w,a,s,d,q,e,shift,space\nSelect objects: keys 1 to 5\n");
		/*Ui::printf("Mover: Pos(%.2f %.2f %.2f) Angs(%.2f %.2f %.2f)", mover->translationWspace.x, mover->translationWspace.y, mover->translationWspace.z,
								 toDegrees(Euler(mover->rotationWspace).x), toDegrees(Euler(mover->rotationWspace).y), toDegrees(Euler(mover->rotationWspace).z));*/

		if(app->getInput().keys[SDL_SCANCODE_ESCAPE])
			break;
		if(app->getInput().keys[SDL_SCANCODE_F11])
			app->togleFullScreen();
		if(app->getInput().keys[SDL_SCANCODE_F12] == 1)
			app->getMainRenderer().takeScreenshot("gfx/screenshot.jpg");

		/*char str[128];
		static string scrFile = (app->getSettingsPath() / "capt" / "%06d.jpg").string();
		sprintf(str, scrFile.c_str(), app->getMainRenderer().getFramesNum());
		app->getMainRenderer().takeScreenshot(str);*/

		// std stuff follow
		app->swapBuffers();
		if(1)
		{
			//if(app->getMainRenderer().getFramesNum() == 100) app->getMainRenderer().takeScreenshot("gfx/screenshot.tga");
			app->waitForNextFrame();
		}
		else
		{
			if(app->getMainRenderer().getFramesNum() == 5000)
			{
				break;
			}
		}
	}while(true);

	INFO("Exiting main loop (" + boost::lexical_cast<std::string>(App::getTicks() - ticks) + ")");
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
int main(int argc, char* argv[])
{
	try
	{
		new App(argc, argv);
		init();

		mainLoop();

		INFO("Exiting...");
		app->quit(EXIT_SUCCESS);
		return 0;
	}
	catch(std::exception& e)
	{
		ERROR("Aborting: " + e.what());
		//abort();
		return 1;
	}
}
